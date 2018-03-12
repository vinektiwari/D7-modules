<?php
/**
* Ability to authorize and communicate with the Salesforce REST API.
*/
class Salesforce {

    public $response;

    /**
     * Constructor which initializes the consumer to salesforce.
     *
     * @param string $consumer_key
     * @param string $consumer_secret
     */
    public function __construct($consumer_key, $consumer_secret) {
        $this->consumer_key = $consumer_key;
        $this->consumer_secret = $consumer_secret;
        $this->login_url = variable_get('sf_login_url');
    }

    /**
     * Function to setup the curl and response it on requested method.
     *
     * @param string $url
     * @param string $data
     * @param string $headers
     * @param string $method
     */
    public function httpRequest($url, $data, $headers = array(), $method = 'GET') {
        if ($method === 'GET') {
            $data = NULL;
        }

        $options = array(
            'method' => $method,
            'headers' => $headers,
            'data' => $data,
        );
        return drupal_http_request($url, $options);
    }

    /**
     * Function which request the token to salesforce.
     *
     * @param string $url
     * @param string $code
     */
    public function requestToken($url,$code) {
        $url = variable_get('sf_login_url'). '/services/oauth2/token';
        $headers = array(
            'Content-Type' => 'application/x-www-form-urlencoded',
        );
        $data = drupal_http_build_query(array(
            'grant_type' => 'authorization_code',
            'code' => $code,
            'client_id' => variable_get('sf_cosumer_key'),
            'client_secret' => variable_get('sf_cosumer_secret'),
            'redirect_uri' => variable_get('sf_callback_url'),
        ));

        $response = httpRequest($url, $data, $headers, 'POST');
        $data = drupal_json_decode($response->data);
        return $data;
    }

    /**
     * Function to collect response from curl on user information
     *
     * @param string $url
     * @param string $accessToken
     */
    function requestUserInfo($url, $accessToken) {
        $headers = array(
            'Content-type' => 'application/x-www-form-urlencoded',
            'Authorization' => 'Bearer '.$accessToken,
        );
        return httpRequest($url, "", $headers, "GET");
    }

    /**
     * Some global query functions to gather row wise record from database to find users identity
     */
    public function get_uid_from_username($username) {
        return db_query('SELECT uid from {users} WHERE name = :name', [':name' => $username])->fetchField();
    }
    public function get_external_uid_from_username($username) {
        return db_query('SELECT uid FROM {authmap} WHERE authname = :authname AND module = :module',[':authname'=>$username,':module'=>'salesforce_auth',])->fetchField();
    }
    public function get_uid_from_sfuserid($user_id) {
        return db_query('SELECT uid FROM {fs_salesforce_auth} WHERE user_id = :user_id', [':user_id' => $user_id])->fetchField();
    }
    public function update_authmap($name, $uid) {
        return db_update('authmap')->fields(['authname' => $name])->condition('uid', $uid)->execute();
    }
    public function set_userid_from_uid($uid,$user_id) {
        return db_update('fs_salesforce_auth')->fields(['user_id' => $user_id])->condition('uid', $uid)->execute();
    }

    public function _sf_set_salesforce_auth_data($uid,$user_id,$user_type) {
        return db_insert('fs_salesforce_auth')->fields(['uid'=>$uid,'user_id'=>$user_id,'user_type'=>$user_type])->execute();
    }
}
