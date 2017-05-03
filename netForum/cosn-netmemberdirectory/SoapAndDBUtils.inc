<?php namespace NetMemberDirectory;

class SoapAndDBUtils{

    public static $RUNNING_SYNC = 'netmemberdirectory_run_sync';
    public static $STOP_SYNC = 'netmemberdirectory_stop_sync';
    public static $DB_TABLE = 'fusionspan_memberdirectory';

    private static $STATE_MAPPING = [
        'AL'=>'Alabama',
        'AK'=>'Alaska',
        'AZ'=>'Arizona',
        'AR'=>'Arkansas',
        'CA'=>'California',
        'CO'=>'Colorado',
        'CT'=>'Connecticut',
        'DE'=>'Delaware',
        'DC'=>'District of Columbia',
        'FL'=>'Florida',
        'GA'=>'Georgia',
        'HI'=>'Hawaii',
        'ID'=>'Idaho',
        'IL'=>'Illinois',
        'IN'=>'Indiana',
        'IA'=>'Iowa',
        'KS'=>'Kansas',
        'KY'=>'Kentucky',
        'LA'=>'Louisiana',
        'ME'=>'Maine',
        'MD'=>'Maryland',
        'MA'=>'Massachusetts',
        'MI'=>'Michigan',
        'MN'=>'Minnesota',
        'MS'=>'Mississippi',
        'MO'=>'Missouri',
        'MT'=>'Montana',
        'NE'=>'Nebraska',
        'NV'=>'Nevada',
        'NH'=>'New Hampshire',
        'NJ'=>'New Jersey',
        'NM'=>'New Mexico',
        'NY'=>'New York',
        'NC'=>'North Carolina',
        'ND'=>'North Dakota',
        'OH'=>'Ohio',
        'OK'=>'Oklahoma',
        'OR'=>'Oregon',
        'PA'=>'Pennsylvania',
        'RI'=>'Rhode Island',
        'SC'=>'South Carolina',
        'SD'=>'South Dakota',
        'TN'=>'Tennessee',
        'TX'=>'Texas',
        'UT'=>'Utah',
        'VT'=>'Vermont',
        'VA'=>'Virginia',
        'WA'=>'Washington',
        'WV'=>'West Virginia',
        'WI'=>'Wisconsin',
        'WY'=>'Wyoming',
    ];

    public static $displayMapping = [
        "first name" => "name",
        "last name" => "last_name",
        "middle name" => "middle_name",
        "title" => "title",
        "email" => "email",
        "city" => "city",
        "state" => "state",
        "zip" => "zipcode",
        "address1" => "address1",
        "address2" => "address2",
        "address3" => "address3",
        "country" => "country",
        "organization" => "employment_org",
        "name prefix" => "name_prefix",
        "name suffix" => "name_suffix",
        "member code" => "member_code",
        "customer type" => "cst_type",
        "latitude" => "latitude",
        "longitude" => "longitude",

    ];

    //mapping => database field
    private static $mappingForImport = [
        "customer_key" => "external_id",
        "first_name" => "name",
        "middle_name" => "middle_name",
        "last_name" => "last_name",
        "email" => "email",
        "title" => "title",
        "name_prefix" => "name_prefix",
        "name_suffix" => "name_suffix",
        "address_line_1" => "address1",
        "address_line_2" => "address2",
        "address_line_3" => "address3",
        "city" => "city",
        "state" => "state",
        "zipcode" => "zipcode",
        "country" => "country",
        "phone_no" => "phone_no",
        "designation" => "designation",
        "department" => "department",
        "domain_name" => "domain_name",
        "employment_organization" => "employment_org",
        "position" => "position",
        "do_not_publish_online" => "do_not_publish_online",
        "customer_member_flag" => "is_member",
        "receives_benefits_flag" => "recv_benefits",
        "cst_type" => "cst_type",
        "member_code" => "member_code",
    ];
    private static $TABLE_PRIMARY_KEY = 'external_id';
    private static $RECORD_LIMIT = 500;

    private static $MODEL;
    private static $NF;


    private static $geoCodeLimit = false;

    public static function getImportMapping(){
        return self::$mappingForImport;
    }

    private static function replaceStateCodeWithName(&$records){
        foreach($records as $record){

            $stateCode = trim($record-> {self::$displayMapping['state']});
            if(isset(self::$STATE_MAPPING[$stateCode])){ //if there is a mapping replace it
                $record->{self::$displayMapping['state']} = self::$STATE_MAPPING[$stateCode];
            }
        }
    }

    private static function cleanString($str){
        return iconv("UTF-8", "UTF-8//IGNORE", $str);
    }

    private static function sanitizeData(&$arrayIn){
        array_walk($arrayIn, function(&$value){
            $value = self::cleanString($value);
        });
    }

    public static function writeFilesToDB($inputVals, $localFilePath){

//		$successFlag = true;
        if(empty($inputVals)){
            form_set_error('netmemberdirectory_overview', "An error occured during import");
        }

        $record = [];
        ini_set("auto_detect_line_endings", "1");
        $file = fopen($localFilePath, "r");
        if(!$file){
            return "File read error. Make sure you have read access to this file " . $localFilePath;
        }

        $max_e_time = ini_get("max_execution_time");

        fgets($file); // discard first line, need to actually create the mapping from this first line since it
        // has the correct column headings. That way we can remove the ui and have a more seamless experience

        while(($cols = fgetcsv($file))!==false){

            set_time_limit(30);
            if (!empty($cols)){ //skip blank lines if any

                $count = 0;
                foreach($inputVals as $currVal){
                    $record[self::$mappingForImport[$currVal]] = $cols[$count];

                    $count++;

                }


                $queryRes =  self::geoCodeAndWriteToDB($record);
                if($queryRes === false){
//					$successFlag = "One or more database errors occurred";
                    drupal_set_message("An error occured attempting to write the customer with cst_id: "
                        . $record[self::$TABLE_PRIMARY_KEY] . " to the database", "error");
                 }
            }
        }

        set_time_limit($max_e_time);
        return true;
    }


    /**
     * Takes a record, geocode based on the city state zip and country and stores the record in the database
     * @param array $record an associative array of db_field => value of one record that will be stored in the database
     * @param array $addressArr an associative array with the following fields: city, state, zip, country
     * @throws \Exception
     * @throws \InvalidMergeQueryException
     * @return false if there is an error
     */
    public static function geoCodeAndWriteToDB($record){
        module_load_include('inc', 'netmemberdirectory', 'maps.netmemberdirectory');

        self::sanitizeData($record);

        $record["latitude"] = null;
        $record["longitude"] = null;

        //TODO keep track of which records need to be geocoded when the limit is hit. Currently it seems that most users will not hit this limit
        if(!self::$geoCodeLimit) {

            //format the address in a way that is readable for the google maps
            //geolocation api

            $retry = 0;

            do {

                $latLng = MapsHelper::geoCodeRecord($record);
//                $latLng = \NetMemberDirectory\MapsHelper::geocode($address);

                if (!is_array($latLng)) {
                    //will need to do geocoding another day when the quota is refreshed
                    if ($latLng == MapsHelper::OVER_QUERY_LIMIT) {
                        self::$geoCodeLimit = true;
                        //error_log("geocode limit reached");
                    } else if ($latLng == MapsHelper::UNKNOWN_ERROR) {
                        $retry++; //allow 1 retry
                    }
                } else {
                    $record['latitude'] = $latLng['lat'];
                    $record['longitude'] = $latLng['lng'];
                }

            } while ($retry == 1);//allow 1 retry
        }

        if(self::$geoCodeLimit){
            $today = (new \DateTime())->format('m/d/Y');
            variable_set(MapsHelper::LAST_SYNCED, $today);
        }

        if(trim($record['country']) == "USA"){
            $record['country'] = "United States";
        }

        $record['datechanged'] = (new \DateTime())->format('Y-m-d H:i:s');

        //error_log("inserting: " . print_r($record, true));

        return $queryRes = db_merge(self::$DB_TABLE)
            ->key([self::$TABLE_PRIMARY_KEY => $record[self::$TABLE_PRIMARY_KEY]])
            ->fields($record)
            ->execute();


    }


    /**
     * Gets records in the dataTable, for display in the place that we need.
     * @param  $atts
     * @return  array of records if no problems, an error message if there is
     */
    public static function getRecords($atts){
        $groupCol = false;
        $fields = [];
        $tableFields='';
        if(!empty($atts['display_fields'])){
            foreach(explode(',', $atts['display_fields']) as $fieldName){
                $curr = trim($fieldName);
                if(!isset(self::$displayMapping[$curr])){
                    return  'Incorrect field "'
                    . $curr
                    .'" listed in the shortcode attribute "display_fields". Please check documentation for correct fields';
                }

                $fields[]= self::$displayMapping[$curr];
                $tableFields .=self::$displayMapping[$curr].",";
            }
        }

        if(!empty($atts['group_by'])){
            $group = explode(",", $atts['group_by']);
            if(isset($group[1])){ //should only have 1 group by option
                return "Group by field should only have one item listed";
            }

            $groupToAdd = trim($group[0]);
            if(!(isset(self::$displayMapping[$groupToAdd]))){
                return 'Incorrect field "' . $groupToAdd
                .'" listed in the shortcode attribute "group_by". Please check the documentation for the correct fields';
            }

            //add in this field if it is not already specified
            if(($groupCol = array_search(self::$displayMapping[$groupToAdd], $fields)) === false){
                $groupCol = count($fields);
                $fields[] = self::$displayMapping[$groupToAdd];
                $tableFields .=self::$displayMapping[$groupToAdd].",";
            }
        }
        $tableFields = rtrim($tableFields, ',');
        $tableAliasDot = 'fs.';

        $query = db_select(self::$DB_TABLE, 'fs')
            ->fields(/*$fields*/ 'fs', $fields);




        if(!empty($atts['filter'])){
            foreach(explode(',', $atts['filter']) as $filter){
                $filterArr = explode("=",trim($filter));
                $filterKey = trim($filterArr[0]);
                if(!isset(self::$displayMapping[$filterKey])){
                    return 'Incorrect field "'. $filterKey
                    . '" listed in the shortcode attribute "filter". Please check documentation for correct fields';
                }
                $query->condition($tableAliasDot.self::$displayMapping[$filterKey], trim($filterArr[1]));
            }
        }

        if(!empty($atts['like_filter'])){
            foreach(explode(',', $atts['like_filter']) as $filter){
                $filterArr = explode("=",trim($filter));
                $filterKey = trim($filterArr[0]);
                if(!isset(self::$displayMapping[$filterKey])){
                    return 'Incorrect field "'. $filterKey
                    . '" listed in the shortcode attribute "like_filter". Please check documentation for correct fields';
                }
                $query->condition($tableAliasDot.self::$displayMapping[$filterKey], "%".trim($filterArr[1])."%", 'LIKE');
            }
        }


        if(!empty($atts['not_filter'])){
            foreach(explode(',', $atts['not_filter']) as $filter){

                $filterArr = explode("=",trim($filter));
                $filterKey = trim($filterArr[0]);
                if(!isset(self::$displayMapping[$filterKey])){
                    return 'Incorrect field "'. $filterKey
                    . '" listed in the shortcode attribute "not_filter". Please check documentation for correct fields';
                }
                $query->condition($tableAliasDot.self::$displayMapping[$filterKey], trim($filterArr[1]), '<>');
            }
        }


        if($atts['ignore_do_not_display_online'] == 0){
            $query->condition($tableAliasDot.'do_not_publish_online', 0);
        }

        if($atts['display_members_only'] == 1){
            $query->condition($tableAliasDot.'is_member', 1);
        }

        if(intval($atts['limit']) && $atts['limit'] > 0){
            $query->range(0,(int)$atts['limit']);
        }

        //execute query
        $queryResult = $query->execute()->fetchAll();

        if(in_array(self::$displayMapping['state'], $fields)){
            self::replaceStateCodeWithName($queryResult);
        }


        $querySize = count($queryResult);

        if(!empty($atts['order_table_by'])){
            $order = explode(",", $atts['order_table_by']);
            if(isset($order[1])){
                return 'order_table_by should only have one item listed';
            }

            if(!isset(self::$displayMapping[trim($order[0])])){
                return  'Incorrect field "'. $order[0]
                . '" listed in the shortcode attribute "order_table_by". Please check documentation for correct fields';
            }

            $oCount = 0;
            $orderField = self::$displayMapping[trim($order[0])];
            foreach($fields as $fCurr){
                if($fCurr == $orderField){
                    break;
                }
                $oCount++;
            }

            if($oCount < $querySize){
                $queryResult['order_by'] = $oCount;
            }
        }

        $queryResult['length'] = $querySize;
        $queryResult['groupBy'] = ($groupCol === false) ? -1 : $groupCol;
        $queryResult['fieldNames'] = $tableFields;
        return $queryResult;
    }

    /**
     * Splits the current date in half if the startDate and endDate are more than 48 hrs apart
     * @param string $startDate must be an earlier date than the end date
     * @param string $endDate
     * @return string the middle date, or the $endDate
     */
    private static function splitDate($startDate, $endDate){
        if($startDate == $endDate){
            return $endDate;
        }

        //error_log("in split date");

        $start = \DateTime::createFromFormat("m/d/Y",$startDate);
        $start = $start->getTimestamp();

        $end = \DateTime::createFromFormat("m/d/Y", $endDate);
        $end = $end->getTimestamp();

        if($start > $end || $start + 172800 > $end){ //check if the dates were reversed
            // or if the startdate is within 48 hrs (172800 seconds) of the end date
            return $endDate;
        }

        $midpoint = ($start + $end)/2;

        $midpointDate = (new \DateTime())->setTimestamp($midpoint)
            ->format("m/d/Y");

        return $midpointDate;



    }

    /**
     * @param $startDate
     * @param $endDate
     * @return bool|string returns true if sync completed successfully, false if it was stopped and self::$RUNNING_SYNC
     * if the sync is still running
     */
    public static function runSync($startDate, $endDate){
        //error_log("doing sync on startDate: " . $startDate . " End date: " . $endDate);
        $today = (new \DateTime())->format("m/d/Y h:i:sa");
        $syncComplete = true;



        if(variable_get(SoapAndDBUtils::$RUNNING_SYNC, 0) == 1){
             //error_log("running sync already");
            return self::$RUNNING_SYNC;
        }

        variable_set(SoapAndDBUtils::$RUNNING_SYNC, '1');
        try {
            variable_set(\NetMemberDirectory\SoapAndDBUtils::$STOP_SYNC, '1');
            sleep(8); //to allow any other sync to stop so that we only have one sync running
            variable_set(\NetMemberDirectory\SoapAndDBUtils::$STOP_SYNC, '0');

            self::runCstSync($startDate, $endDate);
            self::runOrgSync($startDate, $endDate);

            //netmemberdirectory_last_sync is defined in form.menu.inc
            variable_set('netmemberdirectory_last_sync', $today);

        }catch(\Exception $e){
            drupal_set_message($e->getMessage(), 'error');
            //variable_set(SoapAndDBUtils::$RUNNING_SYNC, '0');
            //error_log("Netmemberdirectory plugin sync error: ". $e->getMessage());
        }finally {
            variable_set(SoapAndDBUtils::$RUNNING_SYNC, '0');

            if(self::shouldNotSync()){
                variable_set(self::$STOP_SYNC, 0); //reset this variable as it is currently 1
                $syncComplete = false;
            }
        }

        return $syncComplete;

    }



    public static function runCstSync($startDate, $endDate){
        //error_log("in runCstSync");

        if(self::shouldNotSync()){
            //error_log("Stopped sync cst 1");
            return;
        }

        $max_e_time = ini_get("max_execution_time");
        self::setNetforumConf();


        //$cstRecords = self::$MODEL->getIndividualChangesByDate($startDate, $endDate);
        //error_log("call ind changes");
        $cstRecords = self::$MODEL->getIndividualChangesByDate($startDate, $endDate);

        if(isset($cstRecords->ind_cst_key)){ //in the case there is only one record the structure is not a list of objects
            //but a single object
            $cstRecords = array($cstRecords);
        }else{
            $cstRecords = (array) $cstRecords;
        }

        $recordCount = count($cstRecords);


        //error_log("Cst Record count: " . $recordCount);
        if($recordCount >= self::$RECORD_LIMIT){
            $midDate = self::splitDate($startDate, $endDate);

            //error_log("Split CST " . $startDate . " " . $midDate . " " . $endDate);

            if($midDate != $endDate ) {
                self::runCstSync($startDate, $midDate);
                self::runCstSync($midDate, $endDate);
                return;
            }
        }


        $count =0;
        foreach ($cstRecords as $cResults) {

            if(self::shouldNotSync()){
                //error_log("Stopping Sync Cst records");
                break;
            }

            set_time_limit(60);
            if ($cResults->ind_delete_flag != 0) {

                //error_log("Removing ind record: " . $count++);
                //remove this record from the database
                db_delete(self::$DB_TABLE)
                    ->condition(self::$TABLE_PRIMARY_KEY, (string)$cResults->ind_cst_key)
                    ->execute();
            } else {
                //error_log("getting customer by key: " . $count++);

                $custInf = self::$MODEL->getCustomerByKey((string)$cResults->ind_cst_key);
                //error_log("recieved customer");

                $data = ['external_id' => (string)$cResults->ind_cst_key, 'name' => (string)$cResults->ind_first_name, 'email' => $cResults->eml_address,
                    'middle_name' => $cResults->ind_mid_name, 'last_name' => $cResults->ind_last_name,
                    'title' => $cResults->ind_title, 'name_prefix' => $cResults->ind_prf_code, 'name_suffix' => $cResults->ind_sfx_code,
                    'address1' => $custInf->AddressLine1, 'address2' => $custInf->AddressLine2,
                    'address3' => $custInf->AddressLine3, 'city' => $custInf->AddressCity,
                    'state' => $custInf->AddressState, 'zipcode' => $custInf->AddressZip, 'country' => $custInf->adr_country,
                    'phone_no' => $cResults->phn_number_complete, 'designation' => $cResults->ind_designation, 'employment_org' => $cResults->cst_org_name_dn,
                    'do_not_publish_online' => $cResults->cst_no_web_flag, 'is_member' => $cResults->cst_member_flag,
                    'recv_benefits' => $cResults->cst_receives_benefits_flag, 'member_code' => $custInf->ProductCode];

//                $queryRes = db_merge(self::$DB_TABLE)
//                    ->key([self::$TABLE_PRIMARY_KEY => $data[self::$TABLE_PRIMARY_KEY]])
//                    ->fields($data)
//                    ->execute();
                $queryRes = self::geoCodeAndWriteToDB($data);

                if ($queryRes === false) {
//				$successFlag = "One or more database errors occurred";
                    drupal_set_message("An error occured attempting to write the customer with cst_key: "
                        . $custInf->ind_cst_key . " to the database", "error");
                }

            }
            //sleep for .25 seconds
            //usleep(250000);
        }

        //error_log("exited loop");
        set_time_limit($max_e_time);

    }

    public static function runOrgSync($startDate, $endDate){
        //error_log("in runOrgSync");

        if(self::shouldNotSync()){
            //error_log("Stopped sync org");
            return;
        }


        $max_e_time = ini_get("max_execution_time");
        self::setNetforumConf();


        $cstRecords = self::$MODEL->getOrganizationChangesByDate($startDate, $endDate);


        if(isset($cstRecords->org_cst_key)){ //in the case there is only one record the structure is not a list of objects
                                            //but a single object
            $cstRecords = array($cstRecords);
        }else{
            $cstRecords = (array) $cstRecords;
        }

        $recordCount = count($cstRecords);
        //error_log("Org records Count: " . $recordCount);
        $count = 0;
        if($recordCount >= self::$RECORD_LIMIT){

            $midDate = self::splitDate($startDate, $endDate);
            //error_log("Split ORG " . $startDate . " " . $midDate . " " . $endDate);
            if($midDate != $endDate ) {
                self::runOrgSync($startDate, $midDate);
                self::runOrgSync($midDate, $endDate);
                return;
            }
        }

        foreach ($cstRecords as $currRes) {

            if(self::shouldNotSync()){
                //error_log("Stopping Sync Org records");
                break;
            }

            set_time_limit(60);

            if ($currRes->org_delete_flag != 0) {
                db_delete(self::$DB_TABLE)
                    ->condition(self::$TABLE_PRIMARY_KEY, (string)$currRes->org_cst_key)
                    ->execute();

                //error_log("Removing Organization record: " . $count++);
            } else {

                $fullObj = self::$MODEL->getCustomerByKey($currRes->org_cst_key);
                //error_log("Got org: " . $count++ . " NAME: " . $currRes->cst_sort_name_dn);

                $data = [
                    'external_id' => $currRes->org_cst_key, /*$currRes->org_name,*/
                    'email' => $currRes->eml_address,
                    'address1' => $fullObj->AddressLine1, 'address2' => $fullObj->AddressLine2, 'address3' => $fullObj->AddressLine3,
                    'city' => $fullObj->AddressCity, 'state' => $fullObj->AddressState, 'zipcode' => $fullObj->AddressZip,
                    'country' => $fullObj->adr_country, 'phone_no' => $currRes->phn_number_complete, 'employment_org' => $currRes->cst_sort_name_dn,
                    'do_not_publish_online' => $currRes->cst_no_publish_flag, 'is_member' => $currRes->cst_member_flag,
                    'recv_benefits' => $fullObj->cst_receives_benefits_flag, 'cst_type' => $currRes->cst_type, 'member_code' => $fullObj->ProductCode];

                $queryRes = self::geoCodeAndWriteToDB($data);


                if ($queryRes === false) {
                    drupal_set_message("An error occured attempting to write the customer with org_key: "
                        . $currRes->org_cst_key . " to the database", "error");
                }
            }
            //sleep for .25 seconds
            //usleep(250000);

        }
        set_time_limit($max_e_time);
    }


    private static function setNetforumConf(){
        if(!isset(self::$NF)) {
            self::$NF = _netauth_setconf('', '');
            self::$MODEL = self::$NF->make('Model');
        }
    }


    private static function shouldNotSync(){
        static $stopFlag = false;
        if(!$stopFlag){
            $res = self::getVariableFromDBEvery10Calls(self::$STOP_SYNC, 0);
            //error_log("Shouldsync| stopsync res: " . $res);
            if($res == 1){
                $stopFlag = true;
                return true;
            }
        }
        return $stopFlag;
    }

    /**
     * Alternates between db calls and reading from the drupal cache
     * for performance reasons. Every 10th call (including the very first call)
     * will actually query the database. NOTE: The calls are counted regardless of which
     * function calls it. So it is the aggregate times the function is called and not
     * how many times the calling function called it.
     *
     * @param $name
     * @param $default
     * @return mixed the value of the variable
     */
    private static function getVariableFromDBEvery10Calls($name, $default){
        static $currCount = 0;

        $cacheCount = 10;

        if($currCount++ % $cacheCount == 0){
            return self::getVariableFromDB($name, $default);
        }else{
            return variable_get($name, $default);
        }

    }

    /**
     * Gets a variable by directly querying the database. It avoids the caching of
     * drupals variable_get() function
     * @param $name
     * @param $default
     * @return mixed the value of the variable
     */
    private static function getVariableFromDB($name, $default){
        $query = db_select('variable', 'var')
                ->fields('var', ['name', 'value'])
                ->condition('var.name', $name)
                ->execute()
                ->fetchAssoc();

        if(empty($query)){
            return $default;
        }

        return  unserialize($query['value']);
    }

}