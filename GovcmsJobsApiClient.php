<?php

use GuzzleHttp\Client;

class GovcmsJobsApiClient {
  private $username;
  private $password;
  private $client;
  private $token;
  private $cookie;

  public function __construct($base_uri,$username, $password, $authorization = null) {
    $this->username = $username;
    $this->password = $password;
    $headers = ['Content-Type' => 'application/json'];
    if ($authorization) {
      $headers['Authorization'] = $authorization;
    }
    $this->client = new Client(['base_uri' => $base_uri, 'headers' => $headers]);
  }

  public function login() {

    $token = isset($_SESSION['api_token']) ? $_SESSION['api_token'] : null;
    $cookie = isset($_SESSION['api_cookie']) ? $_SESSION['api_cookie'] : null;
    if (!empty($token) && !empty($cookie)) {
      $this->token = $token;
      $this->cookie = $cookie;
      return TRUE;
    }

    $account_info = sprintf('{
        "username": "%s",
        "password": "%s"
    }', $this->username, $this->password);
    try {
      $response = $this->client->request('POST', '/api/user/login',
        [
          'body' => $account_info,
        ]
      );
    }
    catch (\Exception $e) {
      // \Drupal::messenger()->addError($e->getMessage());
      drupal_set_message($e->getMessage(),'error');
      return FALSE;
    }


    if ($response->getStatusCode() == '200') {
      $body = json_decode((string)$response->getBody());
      if (isset($body->token)) {
        $_SESSION['api_token']=$body->token;
        $this->token = $body->token;
        $cookies = $response->getHeader('Set-Cookie');
        if (isset($cookies[1])) {
          $_SESSION['api_cookie']=$cookies[1];
          $this->cookie = $cookies[1];
        }
        return TRUE;
      }
    }

    return FALSE;
  }

  public function logout() {
    try {
      $response = $this->client->request('POST', '/api/user/logout',
        [
          'headers' => [
            'X-CSRF-Token' => $this->token,
            'cookie' => $this->cookie,
          ],
        ]
      );

      // \Drupal::messenger()->addMessage((string)$response->getBody());
      drupal_set_message($response->getBody());
    }
    catch (\Exception $e) {
      // \Drupal::messenger()->addError($e->getMessage());
      drupal_set_message($e->getMessage(),'error');
    }
    unset($_SESSION['api_token']);
    unset($_SESSION['api_cookie']);
    $this->token = NULL;
    $this->cookie = NULL;
  }

  public function getVacancy($nid) {
    if (!$this->login()) {
      drupal_set_message('Can not login to APS API.','error');
      return FALSE;
    }
    $path = '/api/vacancy/' . $nid;
    try {
      $response = $this->client->request('GET', $path,
        [
          'headers' => [
            'X-CSRF-Token' => $this->token,
            'cookie' => $this->cookie,
          ],
        ]
      );
    }
    catch (\Exception $e) {
      drupal_set_message($e->getMessage(), 'error');
      return FALSE;
    }
    if ($response->getStatusCode() == '200') {
      $vacancy = json_decode((string)$response->getBody());
      return $vacancy;
    }
    return [];
  }

  public function getTaxonomies() {
    if (!$this->login()) {
      drupal_set_message('Can not login to APS API.', 'error');
      return FALSE;
    }
    $path = '/api/get_taxonomies/?machine_name=*';
    try {
      $response = $this->client->request('GET', $path,
        [
          'headers' => [
            'X-CSRF-Token' => $this->token,
            'cookie' => $this->cookie,
          ],
        ]
      );
    }
    catch (\Exception $e) {
      drupal_set_message($e->getMessage(), 'error');
      return FALSE;
    }
    if ($response->getStatusCode() == '200') {
      $taxonomies = json_decode((string)$response->getBody());
      return $taxonomies;
    }
    return [];
  }

  public function getVacancies($offset = null, $limit = null, $options = null)
  {
    if (!$this->login()) {
      \Drupal::messenger()->addError('Can not login to APS API.');
      return false;
    }
    $path = '/api/vacancy/';
    try {
      $response = $this->client->request(
        'GET',
        $path,
        [
          'headers' => [
            'X-CSRF-Token' => $this->token,
            'cookie' => $this->cookie,
          ],
        ]
      );
    } catch (\Exception $e) {
      drupal_set_message($e->getMessage(), 'error');
      return false;
    }
    if ($response->getStatusCode() == '200') {
      $vacancy = json_decode((string)$response->getBody());
      return $vacancy;
    }
    return [];
  }

  public function getAgencies($offset = null, $limit = null, $options = null)
  {
    if (!$this->login()) {
      \Drupal::messenger()->addError('Can not login to APS API.');
      return false;
    }
    $path = '/api/agency/';
    try {
      $response = $this->client->request(
        'GET',
        $path,
        [
          'headers' => [
            'X-CSRF-Token' => $this->token,
            'cookie' => $this->cookie,
          ],
        ]
      );
    } catch (\Exception $e) {
      drupal_set_message($e->getMessage(), 'error');
      return false;
    }
    if ($response->getStatusCode() == '200') {
      $agency = json_decode((string)$response->getBody());
      return $agency;
    }
    return [];
  }

  public function update_vacancy($aps_id,$json_vacancy)
  {
    if (!$this->login()) {
      drupal_set_message('Can not login to APS API.', 'error');
      return false;
    }
    try {
      $response = $this->client->request(
        'PUT',
        '/api/vacancy/'.$aps_id,
        [
          'headers' => [
            'X-CSRF-Token' => $this->token,
            'cookie' => $this->cookie,
          ],
          'body' => $json_vacancy,
        ]
      );
    } catch (\Exception $e) {
      // \Drupal::messenger()->addError($e->getMessage());
      drupal_set_message($e->getMessage(), 'error');
      return false;
    }
    return [];
  }


  public function create_vacancy($json_vacancy)
  {
    if (!$this->login()) {
      drupal_set_message('Can not login to APS API.', 'error');
      return false;
    }
    try {
      $response = $this->client->request(
        'POST',
        '/api/vacancy',
        [
          'headers' => [
            'X-CSRF-Token' => $this->token,
            'cookie' => $this->cookie,
          ],
          'body' => $json_vacancy,
        ]
      );
    } catch (\Exception $e) {
      // \Drupal::messenger()->addError($e->getMessage());
      drupal_set_message($e->getMessage(), 'error');
      return false;
    }
    return json_decode((string)$response->getBody());
  }
}
