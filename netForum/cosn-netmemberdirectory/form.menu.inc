<?php
namespace NetMemberDirectory;


/**
 * Holds the different menu configurations
 *
 */
class Menu{

	public static $NEVER = 'Never';
	public static $DAILY = 'Daily';
	public static $HOURS10 = 'Every 10 Hours';
	public static $HOURS6 = 'Every 6 Hours';
	public static $HOUR = 'Every Hour';
	public static $MINUTES30 = 'Every 30 Minutes';

    //TODO: Add required fields and checks for them
	//private static $requiredFields = ["customer_key", "email_address", "do_not_publish_online"];

	
	public static function menu(){
		
		$items['netmemberdirectory/ajax'] = [
			'title' => 'Ajax Response',
			'page callback' => '_netmemberdirectory_maps_ajax',
			'access arguments' => ['access content'],
			'type' => MENU_CALLBACK,
		];
		
		$items['admin/config/netforum/netmemberdirectory/syncsettings'] = [
				'title' => 'netFORUM Member Directory Sync Settings',
				'description' => 'Member Directory Import From File',
				'access arguments' => ['administer site configuration'],
				'page callback'    => 'drupal_get_form',
				'page arguments'   => ['_netmemberdirectory_sync_settings'],//['netmemberdirectory_admin_settings_form'],
				'weight'		   => 2,
		];
		
		$items['admin/config/netforum/netmemberdirectory/settings'] = [
			'title' => 'Import netFORUM Member Directory',
			'description' => 'Member Directory Import From File',
			'access arguments' => ['administer site configuration'],
			'page callback'    => 'drupal_get_form',
			'page arguments'   => ['netmemberdirectory_manage'],//['netmemberdirectory_admin_settings_form'],
			'weight'		   => 3,
		];
		
		
		return $items;
	}
	
	public static function sync_settings_form(){
		$form['netmemberdirectory_sync'] = [
				'#markup' => t('Manage netFORUM Member Directory Sync Settings'),
				'#prefix' => '<p>',
				'#suffix' => '</p>',
		];

        $lastSynced = variable_get('netmemberdirectory_last_sync', self::$NEVER);

        $form['netmemberdirectory_last_sync'] = [
          '#markup' => '<p>Last Synced: '.$lastSynced.'</p>',
        ];

		$form['netmemberdirectory_sync_status'] = [
			'#markup' => '<p>' . (variable_get(SoapAndDBUtils::$RUNNING_SYNC, 0) == 1 ? 'Sync in Progress' : 'No Sync Running') . '</p>',
		];

		$form['netmemberdirectory_user_stop_sync'] = [
			'#value' => t('Stop Running Sync'),
			'#type' => 'submit',
			'#submit' => ['_netmemberdirectory_stop_sync'],
		];

		$form['netmemberdirectory_sync_freq'] = [
				'#title'         => t('Sync Frequency'),
				'#description'   => t('How often do you want drupal to sync records from netFORUM'),
				'#type'          => 'select',
				'#options'		 => drupal_map_assoc([t(self::$NEVER),
						t(self::$DAILY), t(self::$HOURS10), t(self::$HOURS6), t(self::$HOUR),
						t(self::$MINUTES30)

				]),
				'#default_value' => variable_get('netmemberdirectory_sync_freq', self::$NEVER),
		];
		
		$form['netmemberdirectory_maps_api_key'] = [
				'#title'         => t('Google Maps API Key'),
				'#description'   => t('Enter your Google Map API key in order to use the Google Maps display'),
				'#type'          => 'textfield',
				'#default_value' => variable_get('netmemberdirectory_maps_api_key', ''),
		];
		
		$form['netmemberdirectory_sync_date'] = [
				'#title' => 'Run a Manual Sync',
				'#description' => 'Run a Manual Sync',
				'#type' => 'fieldset',
				'#collapsible' => TRUE,
				'#collapsed' => TRUE,
		];
		
		$form['netmemberdirectory_sync_date']['netmemberdirectory_start_date'] = [
				'#title' => t('Choose the start date you want to sync records for'),
// 				'#description' => t('Choose the date whose records you want to sync'),
				'#type' => 'date',
		];
		
		$form['netmemberdirectory_sync_date']['netmemberdirectory_end_date'] = [
				'#title' => t('Choose the end date you want to sync records for'),
				// 				'#description' => t('Choose the date whose records you want to sync'),
				'#type' => 'date',
		];
		
		$form['netmemberdirectory_sync_date']['netmemberdirectory_sync_submit'] = [
				'#value' => t('Run Sync'),
				'#type' => 'submit',
				'#validate' => ['_netmemberdirectory_manual_sync_validate'],
				'#submit' => ['_netmemberdirectory_run_manual_sync'],
		];
		
		return system_settings_form($form);
	}
	
	public static function manualSyncValidate($form, &$form_state){
		if(!isset($form_state['values']['netmemberdirectory_start_date'])
				|| !isset($form_state['values']['netmemberdirectory_end_date'])){
			form_set_error('netmemberdirectory_sync_date', 'Please enter a valid date.');
		}else{
			$startDateString = $form_state['values']['netmemberdirectory_start_date']['month']
					.'/'.$form_state['values']['netmemberdirectory_start_date']['day']
					.'/'.$form_state['values']['netmemberdirectory_start_date']['year'];
			
			$endDateString = $form_state['values']['netmemberdirectory_end_date']['month']
					.'/'.$form_state['values']['netmemberdirectory_end_date']['day']
					.'/'.$form_state['values']['netmemberdirectory_end_date']['year'];
			
			$startDate = strtotime($startDateString);
			$endDate = strtotime($endDateString);
			
			$today = strtotime(date('m/d/y'));
			
			if($startDate === false || $startDate > $today){
				form_set_error('netmemberdirectory_start_date', 'Please enter a valid start date');
			}
			
			if($endDate === false || $endDate > $today + 86400){ // today + 24hrs
				form_set_error('netmemberdirectory_end_date', 'Please enter a valid end date');
			}
			
			if($startDate > $endDate){
				form_set_error('netmemberdirectory_start_date', 'Start date is after end date');
			}
			
			$form_state['values']['start_date'] = $startDateString;
			$form_state['values']['end_date'] = $endDateString;
		}

	}
	
	public static function runManualSync($form, &$form_state){
		module_load_include('inc','netmemberdirectory','SoapAndDBUtils');

		$syncComplete = SoapAndDBUtils::runSync($form_state['values']['start_date'],
				$form_state['values']['end_date']);

		if($syncComplete === true) {
			drupal_set_message("Member Directory Sync Complete");
		}else if($syncComplete == SoapAndDBUtils::$RUNNING_SYNC) {
			drupal_set_message("Sync already running, please wait until it completes or stop it before proceeding	", 'warning');
		}else{
			drupal_set_message("Member Directory Sync Stopped. Please retry if this was not intentional.", 'warning');
		}
	}

	public static function stopSync($form, &$form_state){
		module_load_include('inc','netmemberdirectory','SoapAndDBUtils');
		variable_set(SoapAndDBUtils::$STOP_SYNC, '1'); //notifies the syncing function that it should stop the sync
		sleep(8);
		variable_set(SoapAndDBUtils::$RUNNING_SYNC, 0);
	}
	
	public static function admin_settings_form(){
		
		$form['netmemberdirectory_overview'] = [
				'#markup' => t('Manage netFORUM Member Directory Settings.'),
				'#prefix' => '<p>',
				'#suffix' => '</p>',
		];
		
// 		$form['netmemberdirectory_sync_freq'] = [
// 				'#title'         => t('Sync Frequency'),
// 				'#description'   => t('How often do you want drupal to sync records from netFORUM'),
// 				'#type'          => 'select',
// 				'#options'		 => drupal_map_assoc([t(self::$NEVER),
// 						t(self::$DAILY), t(self::$HOURS6), t(self::$HOUR),
// 						t(self::$MINUTES30)
		
// 				]),
// 				'#default_value' => variable_get('netmemberdirectory_sync_freq', self::$NEVER),
// 		];
		
// 		$form['file_import_options'] = [
// 				'#title' => 'Import Member Directory From File',
// 				'#description' => 'Import Member Directory Information from a file',
// 				'#type' => 'fieldset',
// 				'#collapsible' => TRUE,
// 				'#collapsed' => TRUE,
// 		];
		
		$count = 1;		
		$form['#tree'] = TRUE;
		
		module_load_include('inc','netmemberdirectory','SoapAndDBUtils');
		$mappingForImport = \NetMemberDirectory\SoapAndDBUtils::getImportMapping();
		
		foreach ($mappingForImport as $id => $currField ){
			
			
			$form['slides'][$count]['id'] = [
					'#type' => 'hidden',
					'#default_value' => $count,
			];
			
			$form['slides'][$count]['field_name'] = [
					'#type' => 'hidden',
					'#default_value' => $id,
			];
			
			$form['slides'][$count]['field'] = [
				'#markup' => t($currField),
			//	'#prefix' => '<p>',
			//	'#suffix' => '</p>',
			];
			
			$form['slides'][$count]['ignore_value'] = [
					'#type' => 'checkbox',
					//'#title' => t('Ignore Value?'),
			];
			
			$form['slides'][$count]['weight'] = [
					'#type' => 'weight',
		            '#title' => t('Weight'),
		            '#title_display' => 'invisible',
		            '#default_value' => $count,
					'#delta' => 30,
			];
			
			$count++;
		}
		
		$form['import_file'] = [
				'#type' => 'file',
				'#title' => t('Upload file (.csv) to import'),
				'#element_validate' => ['_netmemberdirectory_validate_file'],
				'#upload_location' => 'public://tmp/',
				'#upload_validators' => [
						'file_validate_extensions' => ['csv'],
				
				],
		];
		
		$form['importButton'] = [
				'#type' => 'submit',
				'#value' => t('Import File'),
				//'#submit' => ['_netmemberdirectory_test_here'],
				//'#submit' => ['::process_import'],
				
		
		];
		
		/*
		$form['importNetforum'] = [
			'#title' => 'Import Member Directory From netFORUM',
			'#description' => 'Import Member Directory directly from netFORUM',
			'#type' => 'fieldset',
		];
		
		$form['importNetforum']['heading'] = [
				'#markup' => t('Click buttton to import from netFORUM'),
				'#prefix' => '<p>',
				'#suffix' => '</p>',
		];
		
		$form['importNetforum']['webImportButton'] = [
			'#type' => 'submit',
			'#value' => t('Import Directly From netFORUM'),	
			'#submit' => ['_netmemberdirectory_import_from_netforum'],
		];*/ 
		
// 		$form['file'] = array(
// 				'#type' => 'file',
// 				'#title' => t('Image'),
// 				'#description' => t('Upload a file, allowed extensions: jpg, jpeg, png, gif'),
// 		);
		
// 		$form['submit'] = array(
// 				'#type' => 'submit',
// 				'#value' => t('Submit'),
// 				'#submit' => ['_netmemberdirectory_test_here'],
// 		);
		
// 		$form['']
		//$form['file_import_options']['submit'] = array('#type' => 'submit', '#value' => t('Import Files'));
		
		//$form['submit'] = ['#type' => 'submit', '#value' => t('Save Changes')];
		return $form;
	}
	
	
	
	public static function fieldSorter($a, $b){
		if (isset($a['weight']) && isset($b['weight'])) {
			return $a['weight'] < $b['weight'] ? -1 : 1;
		}
		return 0;
	}
	
	public static function manageTheme($variables){
		$form = $variables['form'];
		
		$rows = array();
		foreach (element_children($form['slides']) as $nid) {
			$form['slides'][$nid]['weight']['#attributes']['class'] = array('slides-order-weight');
			$rows[] = array(
					'data' => array(
							array('class' => array('slide-cross')),
							//drupal_render($form['fields'][$nid]['node']),
							drupal_render($form['slides'][$nid]['field']),
							drupal_render($form['slides'][$nid]['ignore_value']),
							drupal_render($form['slides'][$nid]['weight']),
							//drupal_render($form['fields'][$nid]['op']),
					),
					'class' => array('draggable'),
			);
		}
		
		$header = array('', t('field'), t('ignore value?'), t('weight'));
		$output = drupal_render($form['note']);
// 		ini_set('xdebug.var_display_max_depth', -1);
// 		ini_set('xdebug.var_display_max_children', -1);
// 		ini_set('xdebug.var_display_max_data', -1);
		//var_dump($form['file_import_options']);	
		/*$form['#children'] */ $output .= theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('id' => 'slides-order')));
		$output .= drupal_render_children($form);
																	//. drupal_render($form['import_file'])
																	//. drupal_render($form['importButton']);
		
		//var_dump($form['file_import_options']['#children']);
		
		drupal_add_tabledrag('slides-order', 'order', 'sibling', 'slides-order-weight');
		return $output;
	}
	
	public static function validateFile($form, &$form_state){
		$file = file_save_upload('import_file', [
				'file_validate_extensions' => ['csv'],
				//TODO add more validations for csv file passed in
		]);
		
		$form_state['storage']['file'] = $file;
		self::process_import($form, $form_state);
		return;
	}
	
	public static function process_import($form, &$form_state){
		
		module_load_include('inc','netmemberdirectory','SoapAndDBUtils');
		$file =  $form_state['storage']['file'];
		

		if(empty($file)){
			drupal_set_message("Please Upload a File","error");
			return;
		}

		$local_path = drupal_realpath($file->uri);

		if(!usort($form_state['values']['slides'], ['self', 'fieldSorter'])){
			form_set_error('netmemberdirectory_overview', 'An error occurred processing input');
		}
		
		$input_vals = [];
		foreach($form_state['values']['slides'] as $nid => $slide){
			if($slide['ignore_value'] == 0){
				$input_vals[] = $slide['field_name'];
			}
		}
		
// 		var_dump($form_state['values']['slides']);
		$chkVal = \NetMemberDirectory\SoapAndDBUtils::writeFilesToDB($input_vals, $local_path);
		
		if($chkVal === true){
			drupal_set_message("Items were successfully imported");
		}else{
			form_set_error('netmemberdirectory_overview',$chkVal);
		}
	}
} 
