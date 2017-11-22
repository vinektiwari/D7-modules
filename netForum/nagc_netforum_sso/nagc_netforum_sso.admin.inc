<?php
/**
 * @file
 * Administration page callbacks for the nagc neforum_sso module
 */

/**
 * Form builder. Configure nagc_netforum_sso settings.
 *
 * @see system_settings_form()
 */
function nagc_netforum_sso_admin_settings() {

  $form['authentication'] = array(
    '#type'        => 'fieldset',
    '#title'       => t('SSO Settings'),
    '#collapsible' => TRUE,
    '#collapsed'   => FALSE,
  );
  $form['authentication']['nagc_netforum_sso_sso'] = array(
    '#type'          => 'checkbox',
    '#title'         => t('Activate SSO Module'),
    '#default_value' => variable_get('nagc_netforum_sso_sso', NULL),
    '#description'   => t('Make sure you have entered any users that you would like exempt before turning on this module.'),
  );
  $form['authentication']['nagc_netforum_sso_authurl'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Netforum WSDL Authentication web services URL'),
    '#default_value' => variable_get('nagc_netforum_sso_authurl', NULL),
    '#description'   => t('Listed in the Unified Login Web Services Documentation. Should be in the form of http://webportalbridge.[ASSN].org/Authentication.asmx'),
  );
  $form['authentication']['nagc_netforum_sso_xweburl'] = array(
      '#type'          => 'textfield',
      '#title'         => t('Netforum WSDL xWeb OnDemand web services URL'),
      '#default_value' => variable_get('nagc_netforum_sso_xweburl', NULL),
      '#description'   => t('Listed in the Unified Login Web Services Documentation. Should be in the form of http://webportalbridge.[ASSN].org/Authentication.asmx'),
  );
  $form['authentication']['nagc_netforum_sso_authuser'] = array(
      '#type'          => 'textfield',
      '#title'         => t('Security User'),
      '#default_value' => variable_get('nagc_netforum_sso_authuser', NULL),
      '#description'   => t('Security User for calling the Authentication web services'),
  );

  $form['authentication']['nagc_netforum_sso_authpass'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Security Password'),
    '#default_value' => variable_get('nagc_netforum_sso_authpass', NULL),
    '#description'   => t('Security Password for calling the Authentication web services'),
  );
  $form['sso'] = array(
    '#type'          => 'fieldset',
    '#collapsible'   => TRUE,
    '#collapsed'     => TRUE,
    '#title'         => t('Single Sign On Options'),
    '#description'   => t('If Netforum CRM pages will be used instead of the standard Drupal rendered pages, check which of the following should be overriden and if so, enter the link. Or you can remove the links if desired. You must clear the cache before any changes to the menu will be visible.'),
  );
  foreach (nagc_netforum_sso_sso_var() as $key => $var) {
    $form['sso']['nagc_netforum_sso_sso_var_' . $key] = array(
      '#type'          => 'radios',
      '#title'         => t('Override @var Link', array('@var' => $var)),
      '#default_value' => variable_get('nagc_netforum_sso_sso_var_' . $key, 0),
      '#options'       => array(
        t('Use Standard Drupal Link'),
        t('Remove @var Link', array('@var' => $var)),
        t('Override @var Link', array('@var' => $var)),
      ),
    );
    $form['sso']['nagc_netforum_sso_sso_val_' . $key] = array(
      '#type'          => 'textfield',
      '#title'         => t('If overriding, enter URL'),
      '#default_value' => variable_get('nagc_netforum_sso_sso_val_' . $key, ''),
      '#description'   => t('Add http:// if external (ex. http://www.example.com) or do not use preceding slash if internal (ex. node/2).'),
    );
  }
  $form['roles'] = array(
    '#type'          => 'fieldset',
    '#collapsible'   => TRUE,
    '#collapsed'     => TRUE,
    '#title'         => t('Role Mapping'),
    '#description'   => t('Indicate netFORUM role names to Drupal role mapping.'),
  );

  foreach (user_roles(TRUE, NULL) as $key => $var) {
    $form['roles']['nagc_netforum_sso_role_val_' . $key] = array(
      '#type'          => 'textarea',
      '#title'         => t('netFORUM Role Name to map to Drupal role - @var', array('@var' => $var)),
      '#default_value' => variable_get('nagc_netforum_sso_role_val_' . $key, ''),
      '#description'   => t('netFORUM roles to map to Drupal roles. One role per line.')
    );
  }
  $form['exempt'] = array(
    '#type'          => 'fieldset',
    '#collapsible'   => TRUE,
    '#collapsed'     => TRUE,
    '#title'         => t('Exempt Users'),
    '#description'   => t('Indicate what users should be exempt from the SSO process'),
  );
  $form['exempt']['nagc_netforum_sso_exempt'] = array(
    '#type'          => 'textarea',
    '#title'         => t('Exempt users'),
    '#default_value' => variable_get('nagc_netforum_sso_exempt', ''),
    '#description'   => t('List of Drupal User IDs on this site to exempt from Netforum authentication.
                           Enter one user ID per line. These users will login using their Drupal credentials,
                           and can have roles in addition to "authenticated user".  The user ID of a user can be
                           determined by visiting !link and clicking on the "edit" link for each user.  The user ID will
                           then be visible in the URL in the format /user/##/edit where ## is the user ID.', array('!link' => l(t('admin/user/user'), 'admin/user/user'))),
  );

  $form['token'] = array(
    '#type'          => 'fieldset',
    '#collapsible'   => TRUE,
    '#collapsed'     => TRUE,
    '#title'         => t('Token Information'),
    '#description'   => t('How to include stored token on links to netFORUM:'),
  );
  $form['token']['information'] = array(
    '#markup' => '<p>To include the link to netFORUM pages, add either the & or ? symbols and then the token [current-user:netforum] at the end of the URL. The token [current-user:netforum] will be replaced with string "ssotoken=" and then the user stored SSO token number. For example if there are no other variables present in the URL use ?, http://domain.com/?[current-user:netforum] will be automatically replaced with http://domain.com/?ssotoken=0123456789. If there are other variables present in the URL use &, then http://domain.com/eventid=E56890&[current-user:netforum] will be replaced with http://domain.com/eventid=E56890&ssotoken=012345678. This token can be added to content in the WYSIWYG editor or in the menu administration.</p>'
  );
    // Token tree display STARTS
    if (module_exists('token')) {
        $form['token']['token_tree'] = array(
            '#theme' => 'token_tree',
            '#token_types' => array('current-user'),
        );
    }
    else {
        $form['token']['token_tree'] = array(
            '#markup' => '<p>' . t('Enable the <a href="@drupal-token">Token module</a> to view the available token browser.', array('@drupal-token' => 'http://drupal.org/project/token')) . '</p>',
        );
    }
// Token tree display ENDS

  $form = system_settings_form($form);
  $form['#validate'][] = 'nagc_netforum_sso_admin_form_validate';
  return $form;
}

/**
 * Provide some validation of the module settings form to make sure it is a valid url
 *
 * @param $form
 * Nested array of form elements that comprise the form.
 *
 * @param $form_state
 * A keyed array containing the current state of the form. The arguments that drupal_get_form()
 * was originally called with are available in the array $form_state['build_info']['args'].
 *
 */
function nagc_netforum_sso_admin_form_validate($form, &$form_state) {
  $urls = array(
    $form_state['values']['nagc_netforum_sso_authurl'],
    $form_state['values']['nagc_netforum_sso_xweburl'],
  );

  foreach($urls as $url) {
    $parsedurl = parse_url($url);
    nagc_netforum_sso_check_url($parsedurl);
  }

}

/**
 * Checks format of web service URL
 *
 * @param $url
 * The url of the web service to be tested
 */
function nagc_netforum_sso_check_url($url) {
  $scheme = $url['scheme'];
  if (!$scheme) {
    form_set_error($url, t('Missing url scheme. Include http:// or https:// as part of the url'));
  }
  if (($scheme != 'http') && ($scheme != 'https')) {
    form_set_error($url, t('Invalid url scheme. Must be one of http or https'));
  }
}

/**
 * List of Drupal functions to override
 */
 function nagc_netforum_sso_sso_var() {
   return array(
     'ForgotPass' => 'Forgot Password',
      'Register'   => 'Register New Account',
      'Profile'    => 'My Account/Edit Profile',
    );
  }