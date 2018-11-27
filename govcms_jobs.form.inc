<?php

/**
 * @param $form
 * @param $form_state
 * @param null $check_edit
 * @return array
 *
 *
 */
function govcms_jobs_add_form($form, &$form_state,$check_edit=null){
  drupal_add_js('https://cdnjs.cloudflare.com/ajax/libs/select2/4.0.6-rc.0/js/select2.min.js', 'external');
  drupal_add_css('https://cdnjs.cloudflare.com/ajax/libs/select2/4.0.6-rc.0/css/select2.min.css',array('type'=>'external'));
  drupal_add_js(drupal_get_path('module', 'govcms_jobs') .'/govcms_jobs.js', 'file');
  $form = array();
  $params = drupal_get_query_parameters();
  $values = !empty($params['apsid'])?govcms_jobs_search_jobs_by_apsid($params['apsid']): '';
  if ((!empty($values)) && ($values->current_id == null)){
    $data = json_decode($values->data );
  }

  $form['apsid'] =array(
    '#value'=>!(empty($data->vacancy_id))? $data->vacancy_id:0,
    '#type' =>'value',
  );
  $form['check_edit'] = array(
    '#value' => $check_edit && !(empty($data->vacancy_id))?1:0,
    '#type' => 'value',
  );
  $form['agency'] = array(
    '#title'=>'Agency',
    '#type' => 'fieldset',
    '#collapsible' => true,
    '#collapsed' => true,
  );

  $form['agency']['publication_date'] = array(
    '#type' => 'date_popup',
    '#title' => t('Publication date'),
    '#required' => true,
    '#date_format' => 'd-m-Y',
    '#date_increment' => 1,
    '#date_type' => DATE_DATETIME,
    '#datepicker_options' => array(
      'minDate' => '0',
      'maxDate' => '+12 months',
    ),
    '#default_value'=>!empty($data->publication_date->date)?$data->publication_date->date: null,
  );

  $form['agency']['broadband'] = array(
    '#type'=>'checkbox',
    '#title' => 'Broadband',
    '#default_value' => !empty($data->broadband)? $data->broadband:0,
  );

  // change job_level form aps_id => current_id
  if (!empty($data->job_level)){
    $array_level = govcms_jobs_convert_term_apsid_to_current_id($data->job_level);
  }
  else{
    $array_level = array();
  }
  $form['agency']['job_levels'] = array(
    '#type' => 'checkboxes',
    '#title' => 'Job Classification',
    '#options' => govcms_jobs_get_taxonomy_terms('job_levels'),
    '#required' => true,
    '#default_value' => $array_level,
  );
  $array_agencies = db_select('govcms_jobs_mapping','m')
  ->condition('type','agency')
  ->fields('m')
  ->execute()
  ->fetchAll();
  $options = array();
  foreach ($array_agencies as $key=>$value) {
    $data_2 = json_decode($value->data);
    $options[$value->aps_id] = $data_2->title;
  }


  $default_agency = array();
  if (!empty($data->agency)){
    $vacancy_agency = db_select('govcms_jobs_mapping', 'm')
      ->condition('type', 'agency')
      ->condition('aps_id', $data->agency, 'IN')
      ->fields('m')
      ->execute()
      ->fetchAll();
    foreach ($vacancy_agency as $value_agency) {
      $default_agency[] = $value_agency->aps_id;
    }
  }

  $form['agency']['employer'] = array(
    '#type' => 'select',
    '#title' => 'Department/Agency',
    '#options' => $options,
    '#multiple' => TRUE,
    '#required' => true,
    '#default_value' => $default_agency,
  );

  $form['agency']['considered_rr'] = array(
    '#type' => 'radios',
    '#options' => array(
      '1' => t('This box should be selected only if the vacancy being advertised <b>is not</b> a position in which the occupant will be employed under the Public Service Act 1999.'),
      '2' => t('<b>Or</b> I confirm that the agency has considered clients from the Redeployment Register or SES Redeployment Register in relation to this vacancy.'),
    ),
    '#default_value' => !empty($data->considered_rr)? $data->considered_rr:1,
  );

  $form['position_details'] = array(
    '#title' => 'Position Details',
    '#type' => 'fieldset',
    '#collapsible' => true,
    '#collapsed' => true,
  );

  $form['position_details']['job_title'] = array(
    '#type' => 'textfield',
    '#title' => t('Job title'),
    '#required' => true,
    '#maxlength' => 128,
    '#default_value' => !empty($data->job_title) ? $data->job_title : '',
  );
  $form['position_details']['job_categories'] = array(
    '#type' => 'select',
    '#title' => t('Job Category'),
    '#multiple' => true,
    '#required' => true,
    // '#description' => ($is_show && !empty($apsjobs_tooltips_vacancy['job_category'])) ? '<a href="" title="' . $apsjobs_tooltips_vacancy['job_category'] . '">i</a>' : '',
    '#options' => govcms_jobs_get_taxonomy_terms('job_categories'),
    '#default_value' => !empty($data->job_category)?govcms_jobs_convert_term_apsid_to_current_id($data->job_category):null,
  );

  $form['position_details']['field_location_raw'] = array(
    '#type' => 'container',
    '#tree' => true,
    '#prefix' => '<div id="field-location-raw-multiple-wrapper">',
    '#suffix' => '</div>',
    'title' => array('#markup' => '<label>Locations <span class="form-required" title="This field is required.">*</span></label>'),
  );

  if (empty($form_state['locations_count'])) {
    $form_state['locations_count'] = !empty($data->location)? count($data->location) :1;
  }

  for ($delta = 0; $delta < $form_state['locations_count']; $delta++) {
    $form['position_details']['field_location_raw'][LANGUAGE_NONE][$delta] = array(
      'value' => array(
        '#type' => 'textfield',
        '#title' => 'Location',
        '#title_display' => 'invisible',
        '#attributes' => array('placeholder' => 'Type suburb/ postcode'),
        '#autocomplete_path' => 'autocomplete/jobs-location',
        '#default_value' => !empty($data->location[$delta])?$data->location[$delta]:'',
      ),
    );
  }
  $form['position_details']['field_location_raw']['actions'] = array(
    '#type' => 'container',
    '#attributes' => array('class' => array('field-location-raw-actions')),
  );
  $form['position_details']['field_location_raw']['actions']['add_more'] = array(
    '#type' => 'submit',
    '#value' => 'Add another location',
    '#submit' => array('govcms_jobs_field_location_raw_add_more_submit'),
    '#limit_validation_errors' => array(),
    '#ajax' => array(
      'callback' => 'govcms_jobs_field_location_raw_add_more_js',
      'wrapper' => 'field-location-raw-multiple-wrapper',
      'effect' => 'fade',
    ),
    '#attributes' => array('class' => array('form-item')),
  );

  $form['position_details']['position_number'] = array(
    '#type' => 'textfield',
    '#title' => t('Position Number'),
    '#required' => true,
    '#maxlength' => 255,
    '#default_value' => !empty($data->position_number) ? $data->position_number : '',
    // '#default_value' => isset($values['position_number']) ? $values['position_number'] : '',
    // '#attributes' => array('tabindex' => 0),
  );

  $form['position_details']['clearance_level'] = array(
    '#type' => 'select',
    '#title' => t('Security Clearance Level'),
    '#required' => false,
    '#multiple' => true,
    // '#description' => ($is_show && !empty($apsjobs_tooltips_vacancy['clearance_level'])) ? '<a href="" title="' . $apsjobs_tooltips_vacancy['clearance_level'] . '">i</a>' : '',
    '#options' => govcms_jobs_get_taxonomy_terms('clearance_levels'),//apsjobs_vacancies_get_clearance_level(),
    // '#default_value' => isset($values['clearance_level']) ? $values['clearance_level'] : '',
    // '#attributes' => array('tabindex' => 0),
    '#default_value' => !empty($data->clearance_level)?govcms_jobs_convert_term_apsid_to_current_id($data->clearance_level):"",
  );


  $form['position_details']['salary_from'] = array(
    '#type' => 'textfield',
    '#title' => t('Minimum Salary'),
    '#maxlength' => 18,
    '#required' => false,
    '#default_value' => !empty($data->salary_from) ? $data->salary_from : '',
  );

  $form['position_details']['salary_to'] = array(
    '#type' => 'textfield',
    '#title' => t('Maximum Salary'),
    '#maxlength' => 18,
    '#required' => false,
    '#default_value' => !empty($data->salary_to) ? $data->salary_to : '',

  );

  $form['position_details']['working_hours'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Working hours'),
    '#required' => true,
    '#options' => govcms_jobs_get_taxonomy_terms('working_hours'),// + apsjobs_vacancies_get_working_hour(),
    // '#default_value' => isset($values['working_hours']) ? $values['working_hours'] : array(),
    // '#description' => ($is_show && !empty($apsjobs_tooltips_vacancy['working_hours']))
      // ? '<a href="" title="' . $apsjobs_tooltips_vacancy['working_hours'] . '">i</a>' : '',
    '#default_value' => !empty($data->working_hours)?govcms_jobs_convert_term_apsid_to_current_id($data->working_hours):array(),

  );


  $form['position_details']['engagement_types'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Engagement type'),
    '#required' => true,
    '#options' => govcms_jobs_get_taxonomy_terms('engagement_types'),
    '#default_value' => !empty($data->engagement_types)?govcms_jobs_convert_term_apsid_to_current_id($data->engagement_types): array(),
  );

  $form['position_details']['position_initiatives_and_programs'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Position Initiatives and Programs'),
    '#options' => govcms_jobs_get_taxonomy_terms('position_initiatives_and_programs'),
    '#default_value' => !empty($data->initiatives_and_programs)?govcms_jobs_convert_term_apsid_to_current_id($data->initiatives_and_programs):array(),
  );

  $form['duties_eligibility'] = array(
    '#title' => 'Duties, Eligibility and Notes',
    '#type' => 'fieldset',
    '#collapsible' => true,
    '#collapsed' => true,
  );

  $duties = !empty($data->duties) ? $data->duties : '';
  $form['duties_eligibility']['duties'] = array(
    '#type' => 'text_format',
    '#title' => t('Duties'),
    '#required' => true,
    '#default_value' =>  $duties,
  );

  $eligibility = !empty($data->eligibility) ? $data->eligibility : '';
  $form['duties_eligibility']['eligibility'] = array(
    '#type' => 'text_format',
    '#title' => t('Eligibility'),
    '#required' => false,
    '#default_value' => $eligibility,
  );

  $notes = !empty($data->notes) ? $data->notes : '';
  $form['duties_eligibility']['notes'] = array(
    '#type' => 'text_format',
    '#title' => t('Notes'),
    '#required' => false,
    '#default_value' => $notes,
  );

  $form['notice_details'] = array(
    '#title' => 'Notice Details',
    '#type' => 'fieldset',
    '#collapsible' => true,
    '#collapsed' => true,
  );
  $closing_date = !empty($data->closing_date->date)? $data->closing_date->date : '';
  $form['notice_details']['closing_date'] = array(
    '#type' => 'date_popup',
    '#title' => t('Closing date'),
    '#required' => true,
    '#date_format' => 'H:i D d M, Y',
    '#date_increment' => 1,
    '#date_type' => DATE_DATETIME,
    '#datepicker_options' => array(
      'minDate' => '0',
      'maxDate' => '+12 months',
    ),
    '#date_label_position' => 'none',
    '#default_value' => $closing_date,
  );

  $form['notice_details']['under_7_days_check'] = array(
    '#type' => 'checkbox',
    '#title' => 'Agency Head approved closing date less than 7 calendar days',
    // '#return_value' => 1,
    '#default_value' => !empty($data->under_7_days_check) ? $data->under_7_days_check : '',

  );

  $form['notice_details']['apply_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Apply URL'),
    '#default_value' => !empty($data->apply_url) ? $data->apply_url : '',
    '#attributes' => array('placeholder' => array('Enter URL Link'), 'tabindex' => 0),
  );

  $form['notice_details']['contact_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Contact name'),
    '#required' => true,
    '#default_value' => !empty($data->contact_name) ? $data->contact_name : '',
  );

  $form['notice_details']['contact_number'] = array(
    '#type' => 'textfield',
    '#title' => t('Contact number'),
    '#required' => true,
    '#default_value' => !empty($data->contact_phone) ? $data->contact_phone : '',
  );

  $form['notice_details']['contact_email'] = array(
    '#type' => 'textfield',
    '#title' => t('Contact email'),
    '#default_value' => !empty($data->contact_email) ? $data->contact_email : '',
  );

  $form['admin_info'] = array(
    '#title' => 'Administration Information',
    '#type' => 'fieldset',
    '#collapsible' => false,
    '#collapsed' => false,
  );

  $form['admin_info']['editorial_instruction'] = array(
    '#type' => 'textarea',
    '#title' => 'Editorial instructions:',
    '#default_value' => !empty($data->editorial_instruction) ? $data->editorial_instruction : '',
    '#attributes' => array('tabindex' => 0),
  );

  $form['pre_submit'] = array(
    '#title' => 'Pre submit',
    '#type' => 'fieldset',
    '#collapsible' => true,
    '#collapsed' => true,
  );
  $form['pre_submit']['approver']= array(
    '#type' => 'textfield',
    '#title' => t('Approver'),
    '#default_value' => !empty($data->approvers) ? $data->approvers : '',
    '#required' => false,
  );
  $all_fields_on_my_website = field_info_fields();
  $allowed_values = list_allowed_values($all_fields_on_my_website["field_moderation"]);
  $form['pre_submit']['moderation'] = array(
    '#type' => 'select',
    '#title' => t('Moderation'),
    '#options' => $allowed_values,
    '#required' => false,
    '#default_value' => !empty($data->moderation) ? $data->moderation : '',
  );
  $form['save'] = array(
    '#type' => 'submit',
    '#value' => 'Save',
  );

  return $form;
}

/**
 * @param $form
 * @param $form_state
 *
 */
function govcms_jobs_field_location_raw_add_more_submit($form, &$form_state){
  $form_state['locations_count']++;
  $form_state['rebuild'] = true;
}

/**
 * @param $form
 * @param $form_state
 * @return mixed
 *
 */
function govcms_jobs_field_location_raw_add_more_js($form, $form_state){
  return $form['position_details']['field_location_raw'];
}


/**
 * @param $voc_name
 * @return array
 *
 * Load all taxonomy terms by vocabulary machine name.
 *
 */
function govcms_jobs_get_taxonomy_terms($voc_name)
{
  $voc = taxonomy_vocabulary_machine_name_load($voc_name);
  $tree = taxonomy_get_tree($voc->vid);
  if ($voc->vid == 9) {
  }

  $return = array();
  foreach ($tree as $value) {
    $prefix = '';
    $parent = $value->parents[0];
    while ($parent != 0) {
      $prefix .= '---';
      $parent = $tree[$parent]->parents[0];
    }
    $return[$value->tid] = $prefix . $value->name;
  }

  return $return;
}

// to-do handle these values more elegantly
define('MIN_ANNUAL_SALARY', 20000);
define('MAX_ANNUAL_SALARY', 100000000000000000);

/**
 * @param $form
 * @param $form_state
 *
 */
function govcms_jobs_add_form_validate($form,&$form_state){
  $values = $form_state['values'];
  $now = strtotime(date("d-m-Y"));
  $publication_date = strtotime($values['publication_date']);
  if ($publication_date < $now){
    form_set_error('publication_date','Publication date can not be in the past.');
  }
  if (!is_numeric($values['salary_from']) && !empty($values['salary_from'])) {
    form_set_error('salary_from', 'Minimum salary must be a number.');
  }
  elseif (_number_val($values['salary_from']) < MIN_ANNUAL_SALARY) {
    form_set_error('salary_from', 'Minimum salary must be equal or greater than $' . MIN_ANNUAL_SALARY . '.');
  }
  elseif (_number_val($values['salary_from']) > MAX_ANNUAL_SALARY) {
    form_set_error('salary_from', 'Minimum salary is too big (max value is 10^17).');
  }
  if (isset($values['working_hours']) && isset($values['working_hours']['all'])) {
    unset($values['working_hours']['all']);
  }

  if (isset($values['engagement_types']) && isset($values['engagement_types']['all'])) {
    unset($values['engagement_types']['all']);
  }

  $location_ids = array();
  // Location validate
  $location_inputs = array();
  foreach ($values['field_location_raw'][LANGUAGE_NONE] as $data) {
    $location_inputs[] = $data['value'];
  }
  $empty_location = true;
  foreach ($location_inputs as $delta => $location_input) {
    if (empty($location_input)) {
      continue;
    }
    $lid = govcms_jobs_location_autocomplete_validate($location_input);
    if (!empty($lid)) {
      $location_ids[] = $lid;
    } else {
      form_set_error("field_location_raw][und][$delta][value", t('There are no locations matching "%value"', array('%value' => $location_input)));
    }
    $empty_location = false;
  }
  $form_state['values']['location_ids'] = $location_ids;
  if ($empty_location) {
    form_set_error('field_location_raw][und][0][value', 'Location field is required.');
  }
  $location_clone = array();
  foreach ($values['field_location_raw'][LANGUAGE_NONE] as $data) {
    $location_clone[] = $data['value'];
  }
  $location_unique = array_unique($location_clone);
  if (count($location_clone) != count($location_unique)) {
    form_set_error('field_location_raw', 'Location is duplicated.');
  }

  // notice
  if (isset($values['closing_date'])) {
    if (isset($values['closing_date']['date'])
      && isset($values['closing_date']['time'])) {
      $values['closing_date'] = implode(' ', $values['closing_date']);
    }

    $closing_date = strtotime($values['closing_date']);
    if ($closing_date == false || $closing_date <= $publication_date) {
      form_set_error('closing_date', t('Closing date field is invalid.'));
    }
    if (($closing_date <= $publication_date+7*8640) && (!$values['under_7_days_check']) ) {
      form_set_error('closing_date', t('The closing date is greater than the publication date'));
    }
  }

  $apply_url = $values['apply_url'];
  if (!empty($apply_url)) {
    if (strpos($apply_url, '://') === false) {
      $apply_url = 'http://' . $apply_url;
    }
    if (!__valid_url($apply_url)) {
      form_set_error('apply_url', t('Apply url field is invalid.'));
    }
  }

  // phone module requires patching for update AU phone numbers - it is dated be warned
  $phonenumber = $values['contact_number'];
  module_load_include('inc', 'phone', 'include/phone.au');
  if (function_exists('valid_au_phone_number')) {
    if (!valid_au_phone_number($phonenumber)) {
      form_set_error('contact_number', t(phone_au_metadata()['error'], array('%value' => $phonenumber)));
    }
    if (!validate_phone($phonenumber)) {
      form_set_error('contact_number', $phonenumber . ' is not a valid Australian phone number</br>Australian phone numbers should contain only numbers with an optional prefix of "+61"');
    }
  }

  if (!empty($values['contact_email'])) {
    if (!valid_email_address($values['contact_email'])) {
      form_set_error('contact_email', t('Contact email field is invalid.'));
    }
  }
}

/**
 * @param $url
 * @return int
 *
 * My tweak on a regex for a valid URL
 *
 */
function __valid_url_rod($url)
{
  return preg_match("^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?", $url);
}

/**
 * @param $url
 * @return int
 *
 * More comprehensive regex for valid URL's
 *
 */
function __valid_url($url)
{
  return preg_match('_^(?:(?:https?|ftp)://)(?:\S+(?::\S*)?@)?(?:(?!10(?:\.\d{1,3}){3})(?!127(?:\.\d{1,3}){3})(?!169\.254(?:\.\d{1,3}){2})(?!192\.168(?:\.\d{1,3}){2})(?!172\.(?:1[6-9]|2\d|3[0-1])(?:\.\d{1,3}){2})(?:[1-9]\d?|1\d\d|2[01]\d|22[0-3])(?:\.(?:1?\d{1,2}|2[0-4]\d|25[0-5])){2}(?:\.(?:[1-9]\d?|1\d\d|2[0-4]\d|25[0-4]))|(?:(?:[a-z\x{00a1}-\x{ffff}0-9]+-?)*[a-z\x{00a1}-\x{ffff}0-9]+)(?:\.(?:[a-z\x{00a1}-\x{ffff}0-9]+-?)*[a-z\x{00a1}-\x{ffff}0-9]+)*(?:\.(?:[a-z\x{00a1}-\x{ffff}]{2,})))(?::\d{2,5})?(?:/[^\s]*)?$_iuS', $url);
}

/**
 * @param $input
 * @return bool
 *
 *
 *
 */
function govcms_jobs_location_autocomplete_validate($input)
{
  return govcms_jobs_validate_location_input($input);
}

/**
 * @param $input
 * @return bool
 *
 *
 *
 */
function govcms_jobs_validate_location_input($input){
  $input = trim($input);
  if (empty($input)) {
    return false;
  }
  $parts = explode(', ', $input);
  if (count($parts) == 3) {
    list($locality, $state, $pcode) = $parts;
  } else {
    return false;
  }
  $matches = array();

  if ($input != '') {
    $query = db_select('govcms_jobs_location', 'l');
    $query->fields('l', array('id'));
    $query->condition('postcode', $pcode);
    $query->condition('state_code', $state);
    $query->condition('place_name', $locality);
    $lid = $query->execute()->fetchField();
  }
  return $lid;
}

/**
 * @param $form
 * @param $form_state
 *
 *
 *
 */
function govcms_jobs_field_location_raw_remove_last_submit($form, &$form_state){
  if ($form_state['locations_count'] > 1) {
    $form_state['locations_count']--;
  }
  $form_state['rebuild'] = true;
}

/**
 * @param $num
 * @return mixed
 *
 * Get numberic value of number string
 *
 */
function _number_val($num)
{
  return $num * 1;
}


/**
 * @param $number
 * @return int
 *
 * Regex for phone numbers - phone module somewhat dated now
 *
 */
function validate_phone($number)
{
  $number = preg_replace('/\s+/', "", $number);
  $return = preg_match('/^\({0,1}((0|\+61)(2|4|3|7|8)){0,1}\){0,1}(\ |-){0,1}[0-9]{2}(\ |-){0,1}[0-9]{2}(\ |-){0,1}[0-9]{1}(\ |-){0,1}[0-9]{3}$/', $number);
  return $return;
}

/**
 * @param $id
 * @return mixed
 *
 *
 *
 */
function govcms_jobs_search_jobs_by_apsid($id){
  $query = db_select('govcms_jobs_mapping','m');
  $query->condition('type','job')
        ->condition('aps_id',$id)
        ->fields('m');
  $data = $query->execute()->fetchObject();
  return $data;
}

/**
 * @param $apsids
 * @return mixed
 *
 *
 *
 */
function govcms_jobs_convert_term_apsid_to_current_id($apsids){
  $query = db_select('govcms_jobs_mapping','m');
  $query->condition('type','taxonomy');
  $query->condition('aps_id',$apsids,'IN');
  $query->fields('m',array('current_id'));
  $current_ids = $query->execute()->fetchCol();
  return $current_ids;
}

function govcms_jobs_convert_job_apsid_to_current_id($apsid){
  $query = db_select('govcms_jobs_mapping', 'm');
  $query->condition('type', 'job');
  $query->condition('aps_id', $apsid);
  $query->fields('m', array('current_id'));
  $current_ids = $query->execute()->fetchCol();
  return $current_ids[0];
}

/**
 * @param $form
 * @param $form_state
 * @throws Exception
 *
 *
 *
 */
function govcms_jobs_add_form_submit($form, &$form_state)
{
  $values = $form_state['values'];
  $apsid= $values['apsid'];
  if ($form_state['values']['check_edit']== 0){
    $newNode = (object)null;
    $newNode->type = 'govcms_jobs';
    $newNode->status = 1;
  }
  else{
    $nid = govcms_jobs_convert_job_apsid_to_current_id($apsid);
    $newNode = node_load($nid);
  }

  $newNode->title = $values['job_title'];
  // add CCK field data
  $newNode->field_publication_date['und'][0]['value'] = $values['publication_date'];
  $newNode->field_broadband['und'][] = array('value' => $values['broadband']);
  if (!empty($values['job_levels'])) {
    $newNode->field_job_levels['und']=null;
    foreach ($values['job_levels'] as $key => $value) {
      if ($value != 0) {
        $newNode->field_job_levels['und'][] = array('value' => $value);
      }
    }
  }

  if (!empty($values['employer'])) {
    $newNode->field_agencies['und'] = null;
    foreach ($values['employer'] as $key => $value) {
      if ($value != 0) {
        $newNode->field_agencies['und'][] = array('value' => $value);
      }
    }
  }

  $newNode->field_considered_rr['und'][] = array('value' => $values['considered_rr']);
  $newNode->fiel_job_title['und'][] = array('value' => $values['job_title']);
  if (!empty($values['job_categories'])) {
    foreach ($values['job_categories'] as $key => $value) {
      $newNode->field_job_categories['und']=null;
      if ($value != 0) {
        $newNode->field_job_categories['und'][] = array('value' => $value);
      }
    }
  }
  if (!empty($values['field_location_raw']['und'])) {
    $newNode->field_locations['und'] = null;
    foreach ($values['field_location_raw']['und'] as $key => $value) {
      if ($value != 0) {
        $newNode->field_locations['und'][] = array('value' => strtoupper($value['value']));
      }
    }
  }
  $newNode->field_position_number['und'][0]['value'] = $values['position_number'];

  foreach ($values['clearance_level'] as $key => $value) {
    $newNode->field_clearance_level['und'] = null;
    if ($value != 0) {
      $newNode->field_clearance_level['und'][] = array('value' => $value);
    }
  }


  $newNode->field_salary_min['und'][] = array('value' => $values['salary_from']);
  $newNode->field_salary_max['und'][] = array('value' => $values['salary_to']);

  $newNode->field_working_hours['und'] = null;

  foreach ($values['working_hours'] as $key => $value) {
    if ($value != 0) {
      $newNode->field_working_hours['und'][] = array('value' => $value);
    }
  }


  if (!empty($values['engagement_types'])) {
    $newNode->field_engagement_types['und'] = null;
    foreach ($values['engagement_types'] as $key => $value) {
      if ($value != 0) {
        $newNode->field_engagement_types['und'][] = array('value' => $value);
      }
    }
  }

  if (!empty($values['positions_initiatives_and_programs'])) {
    $newNode->field_initiatives_and_programs['und'] = null;
    foreach ($values['positions_initiatives_and_programs'] as $key => $value) {
      if ($value != 0) {
        $newNode->field_initiatives_and_programs['und'][] = array('value' => $values['positions_initiatives_and_programs']);
      }
    }
  }
  $newNode->field_duties['und'][0]['value'] = $values['duties']['value'];
  $newNode->field_eligibility['und'][0]['value'] = $values['eligibility']['value'];
  $newNode->field_notes['und'][0]['value'] = $values['notes']['value'];
  $newNode->field_closing_date['und'][0]['value'] = $values['closing_date'];
  $newNode->field_under_7_days_check['und'][0]['value'] = $values['under_7_days_check'];
  $newNode->field_apply_url['und'][] = array('value' => $values['apply_url']);
  $newNode->field_contact_name['und'][] = array('value' => $values['contact_name']);
  $newNode->field_contact_number['und'][] = array('value' => $values['contact_number']);
  $newNode->field_contact_email['und'][] = array('value' => $values['contact_email']);


  $newNode->field_editorial_instruction['und'][] = array('value' => $values['editorial_instruction']);

  $newNode->field_moderation['und'][] = array('value' => $values['moderation']);
  $newNode->field_approver['und'][] = array('value' => $values['approver']);

  // save node

  node_save($newNode);
  if ($values['check_edit'] == 0) {
    $updated = db_update('govcms_jobs_mapping')
      ->condition('type','job')
      ->condition('aps_id',$values['apsid'])
      ->fields(array(
        'current_id'=>$newNode->nid,
      ))
      ->execute();

    $json = govcms_jobs_api_json_vacancy($newNode);
    $client = new GovcmsJobsApiClient(variable_get('govcms_jobs_base_uri', ''), variable_get('govcms_jobs_api_username', ''), variable_get('govcms_jobs_api_password', ''), variable_get('govcms_jobs_authorization', ''));
    if ($apsid==0){
      $node_id = $client->create_vacancy($json);
      drupal_set_message('Job ' . $values['job_title'] . ' have been created');
      drupal_set_message('Vacancy ' . $node_id->nid . ' have been created');
    }
    else{
      $data = $client->update_vacancy($apsid, $json);
      drupal_set_message('Job ' . $values['job_title'] . ' have been saved');
    }
  }
  else {
    $json = govcms_jobs_api_json_vacancy($newNode);
    $client = new GovcmsJobsApiClient(variable_get('govcms_jobs_base_uri', ''), variable_get('govcms_jobs_api_username', ''), variable_get('govcms_jobs_api_password', ''), variable_get('govcms_jobs_authorization', ''));
    $data = $client->update_vacancy($apsid,$json);
    drupal_set_message('Job ' . $values['job_title'] . ' have been saved');
  }
}


/**
 * @param $node
 * @return array
 *
 *
 *
 */
function govcms_jobs_api_json_vacancy($node) {
  $json = array();
  $json['type'] = 'aps_vacancies';
  $time = strtotime($node->field_publication_date['und'][0]['value']);
  $publication_date = date ('D M d, Y',$time);
  $json['publication_date']['date'] =  $publication_date;
  $json['broadband']= $node->field_broadband['und'][0]['value'];
  foreach ($node->field_job_levels['und'] as $value) {
    $json['job_level'][] = govcms_jobs_get_apsid_from_current_id('taxonomy',$value['value']);
  }
  foreach ($node->field_agencies['und'] as $value) {
    $json['agency'][] = $value['value'];
  }
  $json['considered_rr'] = $node->field_considered_rr['und'][0]['value'];
  $json['job_title'] = $node->title;
  if (!(empty($node->field_job_categories))){
    foreach ($node->field_job_categories['und'] as $value) {
      $json['job_category'][] = govcms_jobs_get_apsid_from_current_id('taxonomy', $value['value']);
    }
  }
  foreach ($node->field_locations['und'] as $value) {
    $json['locations'][] = $value['value'];
  }
  $json['position_number'] = $node->field_position_number['und'][0]['value'];
  foreach ($node->field_clearance_level['und'] as $value) {
    $json['clearance_level'][] = govcms_jobs_get_apsid_from_current_id('taxonomy', $value['value']);
  }
  $json['salary_from'] = $node->field_salary_min['und'][0]['value'];
  $json['salary_to'] = $node->field_salary_max['und'][0]['value'];
  if (!(empty($node->field_working_hours))){
    foreach ($node->field_working_hours['und'] as $value) {
      $json['working_hours'][] = govcms_jobs_get_apsid_from_current_id('taxonomy', $value['value']);
    }
  }
  if (!(empty($node->field_engagement_types))){
    foreach ($node->field_engagement_types['und'] as $value) {
      $json['engagement_types'][] = govcms_jobs_get_apsid_from_current_id('taxonomy', $value['value']);
    }
  }

  if (!(empty($node->field_initiatives_and_programs))){
    foreach ($node->field_initiatives_and_programs['und'] as $value) {
      $json['initiatives_and_programs'][] = govcms_jobs_get_apsid_from_current_id('taxonomy', $value['value']);
    }
  }
  $json['duties'] = $node->field_duties['und'][0]['value'];
  $json['eligibility'] = $node->field_eligibility['und'][0]['value'] ;
  $json['notes'] =  $node->field_notes['und'][0]['value'];

  $time = strtotime($node->field_closing_date['und'][0]['value']);
  $publication_date = array('date'=>date('D M d, Y', $time),'time'=> date('h:m', $time));
  $json['closing_date'] = $publication_date;
  $json['under_7_days_check'] = $node->field_under_7_days_check['und'][0]['value'];
  $json['under_apply_url'] = $node->field_apply_url['und'][0]['value'];
  $json['contact_name'] = $node->field_contact_name['und'][0]['value'];
  $json['contact_number'] = $node->field_contact_number['und'][0]['value'];
  $json['contact_email'] = $node->field_contact_email['und'][0]['value'];
  if ($node->field_approver['und'][0]['value']){
    $json['approver'] = $node->field_approver['und'][0]['value'];
  }
  else{
    $json['approver'] = 1;
  }

  $json['moderation'] = $node->field_moderation['und'][0]['value'];
  $json=json_encode($json);
  return $json;
}

/**
 * @param $type
 * @param null $id
 * @return string
 *
 *
 *
 */
function govcms_jobs_get_apsid_from_current_id($type, $id=null){
  if ($id == null){
    return '';
  }
  $query = db_select('govcms_jobs_mapping', 'm')
    ->condition('current_id', $id)
    ->condition('type', $type)
    ->fields('m', array('aps_id'))
    ->execute();
  $result = $query->fetchAll();
  return $result[0]->aps_id;
}
