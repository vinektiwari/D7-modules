<?php namespace NetMemberDirectory;

class MapsHelper{
	//geocoding status reponse codes
	const OK = 'OK';
	const OVER_QUERY_LIMIT = 'OVER_QUERY_LIMIT';
	const REQUEST_DENIED = 'REQUEST_DENIED';
	const INVALID_REQUEST = 'INVALID_REQUEST';
	const UNKNOWN_ERROR = 'UNKNOWN_ERROR';

	//our consts
	const NO_ADDRESS = 'NO_ADDRESS';
	const LAST_SYNCED = 'netmemberdirectory_maps_lastsynced'; //location of the last synced variable
	
	//const MAPS_API_KEY = 'AIzaSyBmheHZBBbB3sGmlDxcznixTWJ4_tlBVWg';
	
	public static function getMapsRecordsFromDB($attrs){
		
		$tableHeader = $tableField = null;
		$attrs = shortcode_attrs(array(
				//'class' => 'member_list',
				'limit' => -1,
				'display_members_only' => 1,
				'display_fields' => 'first name, last name, title, email, city, state',
				'ignore_do_not_display_online' => 0,
				'filter' => '',
				'like_filter'=>'',
				'not_filter' => '',
				'like_filter' => '',
				'center' => 'United States',
			),
			$attrs
		);
		
		$attrs ['display_fields'] = "name prefix, name suffix, first name, last name, organization, city, state, zip, country, latitude, longitude, customer type";
		
		module_load_include('inc', 'netmemberdirectory', 'SoapAndDBUtils');
		$recordsArr = \NetMemberDirectory\SoapAndDBUtils::getRecords($attrs);
		
		if(!is_array($recordsArr)){
			return $recordsArr;
		}
		
		
		return json_encode($recordsArr);
	}

	/**
	 * @param string $address the properly formatted address to be converted into a lat
	 * @return array | string if successful returns an associative array with fields "lat" and "lng", otherwise a string with the error
	 */
	public static function geocode($address){
		$address = urlencode($address);
		$url = 'https://maps.googleapis.com/maps/api/geocode/json?address='.$address;

		$geoInfo = json_decode(file_get_contents($url), true);
// 		ini_set('xdebug.var_display_max_depth', -1);
// 		ini_set('xdebug.var_display_max_children', -1);
// 		ini_set('xdebug.var_display_max_data', -1);
//  		var_dump($geoInfo['results'][0]['geometry']['location']);die();
		if(empty($address)){
			return self::NO_ADDRESS;
		}
		usleep(120000); //sleep for ~ 0.12 of a second since there you can only make 10 request per second

		if($geoInfo['status'] == self::OK ){
			return $geoInfo['results'][0]['geometry']['location']; //has two fields lat and lng
		}else{
			return $geoInfo['status'];
		}
	}


	/**
	 * Builds an address in a for that can be geocoded based on a database record
	 * @param array $record
	 * @return array | string if successful returns an associative array with fields "lat" and "lng", otherwise a string with the error
	 */
	public static function geoCodeRecord($record){
		//see SoapAndDBUtils.inc for more information about the fields in the records as these fields
		//should relate to the database fields
		$city = empty($record['city']) ? '' : ($record['city'] . ', ');
		$state = empty($record['state']) ? '' : ($record['state'] . ', ');
		$zip = empty($record['zipcode']) ? '' : ($record['zipcode'] . ' ');
		$country = empty($record['country']) ? '' : ($record['country']);;

		$address = trim( $city.$state.$zip.$country, " \t\n\r\0\x0B,");
		return self::geocode($address);
	}

	/**
	 * Updates the addresses that we could not previously update
	 * as we hit the query limit for these particular addresses
	 */
	public static function cronUpdateAddresses(){
		module_load_include('inc','netmemberdirectory' ,'SoapAndDBUtils');


		$orCond = db_or()
			->isNull('fs.latitude')
			->isNull('fs.longitude');


		$records = db_select(SoapAndDBUtils::$DB_TABLE, 'fs')
				->fields('fs')
				->condition($orCond)
				->execute()
				->fetchAll(\PDO::FETCH_ASSOC);

		variable_set(self::LAST_SYNCED, '');
		module_load_include('inc', 'netmemberdirectory', 'SoapAndDBUtils');
		foreach($records as $cRecord){
			SoapAndDBUtils::geoCodeAndWriteToDB($cRecord);
		}
	}
}
