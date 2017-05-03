<?php

class Soap {
    public $soap_client;
    private $key;
    private $password;
    private $wsdl;
    private $response;
    private $namespace;
    private static $instance = null;
    
    public static function getInstance() {
        if(self::$instance == null) {
            self::$instance = new Soap();
        }
        return self::$instance;
    }

    private function Authenticate(){
        $response = [];
        $result = $this->soap_client->__soapCall("Authenticate", array( "params" => array("userName" => $this->key, "password" => $this->password)),null, null, $response);
        $this->namespace = $result->AuthenticateResult;
        $authToken = $response['AuthorizationToken']->Token;
        return $authToken;
    }

    private function setupSoapHeader(){
        $token = null;
        if ($this->response == null || !isset($this->response['AuthorizationToken']) || !isset($this->response['AuthorizationToken']->Token)){
            $token = $this->Authenticate();
        } else {
            $token = $this->response['AuthorizationToken']->Token;
        }
        return new SoapHeader($this->namespace, "AuthorizationToken", array("Token" => $token));
    }

    public function soapCall($funcName, $params){
        $header = $this->setupSoapHeader();
        $this -> response = [];
        return $this->soap_client->__soapCall($funcName, $params, null, $header, $this->response);
    }
}

$objSoapClass =  new Soap();