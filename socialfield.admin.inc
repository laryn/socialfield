<?php
/**
 * @file
 * All code for administrative pages, left here for performance reasons.
 */

/**
 * Page callback: Table that displays all available services.
 */
function socialfield_settings($form) {
  $config = config('socialfield.settings');

  $form = array(
    '#config' => 'socialfield.settings',
    '#attached' => array(
      'css' => array(backdrop_get_path('module', 'socialfield') . '/css/socialfield.css'),
    ),
  );
  $form['settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Socialfield Settings'),
    '#collapsible' => TRUE,
  );
  $form['settings']['filled'] = array(
    '#type' => 'checkbox',
    '#title' => t('Use filled versions of icons'),
    '#default_value' => $config->get('filled'),
  );

  $header = array(
    t('Service'),
    t('Icon'),
    t('Machine Name'),
    t('Validation pattern'),
    t('Operations'),
  );

  $rows = array();
  $services = $config->get('services');
  if (is_array($services)) {
    foreach ($services as $name => $service) {
      // Sanitize
      $machine_name = check_plain($name);
      $service_name = check_plain($service['name']);
      $icon_name = check_plain($service['icon']);
      $validation_pattern = check_plain($service['validation_pattern']);

      $dropbuttons = array(
        '#type' => 'dropbutton',
        '#links' => array(
          'configure' => array(
            'title' => t('Configure'),
            'href' => 'admin/config/media/socialfield/' . $machine_name . '/edit',
          ),
          'delete' => array(
            'title' => t('Delete'),
            'href' => 'admin/config/media/socialfield/' . $machine_name . '/delete',
          ),
        ),
      );

      if ($config->get('filled')) {
        $icon_name .= '-fill';
      }
      $icon_options = array(
        'attributes' => array(
          'class' => array('socialfield-' . $machine_name),
          'width' => 30,
          'height' => 30,
        ),
      );

      $rows[] = array(
        $service_name,
        icon($icon_name, $icon_options),
        $name,
        str_replace("\n", '<br />', $validation_pattern),
        backdrop_render($dropbuttons),
      );
    }
  }

  $form['services'] = array(
    '#markup' => theme('table', array(
      'header' => $header,
      'rows' => $rows,
      'empty' => t('No social services were identified.'),
    ))
  );

  return system_settings_form($form);
}

/**
 * Form for adding a new service.
 */
function socialfield_add_service($form, $form_state) {
  $form['service_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Service'),
  );
  $form['service'] = array(
    '#type' => 'machine_name',
    '#title' => t('Service machine name'),
    '#machine_name' => array(
      'exists' => 'socialfield_service_exists',
      'source' => array('service_name'),
    ),
  );
  $form['service_icon'] = array(
    '#type' => 'textfield',
    '#title' => t('CSS classes'),
    '#description' => t('Separate multiple classes with spaces. See <a href="https://docs.backdropcms.org/documentation/icon-api">the Backdrop Icon API</a>'),
  );
  $form['service_validation_pattern'] = array(
    '#type' => 'textarea',
    '#title' => t('URL validation pattern'),
    '#description' => t('Enter the list of allowed URL patterns separated by new lines.<br />Leave empty to allow user input any URLs.<br />The "*" character is a wildcard.<br />Example: *youtube.com/* for any page on the YouTube site.'),
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Add'),
  );

  return $form;
}

/**
 * Validates form when adding a new service.
 */
function socialfield_add_service_validate($form, &$form_state) {
  $services = config_get('socialfield.settings', 'services');
  if (isset($services[$form_state['values']['service']])) {
    form_set_error('service', t('This service already exist.'));
  }
}

/**
 * Form submit for adding a new service.
 */
function socialfield_add_service_submit($form, $form_state) {
  $values = &$form_state['values'];
  $services = config_get('socialfield.settings', 'services');

  $service = array(
    'name' => $values['service_name'],
    'icon' => $values['service_icon'],
    'validation_pattern' => $values['service_validation_pattern'],
  );

  $services[$values['service']] = $service;

  config_set('socialfield.settings', 'services', $services);
  backdrop_goto('admin/config/media/socialfield');
}

/**
 * Form for editing a service.
 */
function socialfield_edit_service($form, $form_state, $service = NULL) {
  if (!isset($service)) {
    backdrop_set_message(t('No service specified.'), 'error');
    backdrop_goto('admin/config/media/socialfield');
  }

  $services = config_get('socialfield.settings', 'services');

  if (!isset($services[$service])) {
    backdrop_set_message(t('The specified service does not exist.'), 'error');
    backdrop_goto('admin/config/media/socialfield');
  }

  $form['service_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Service'),
    '#default_value' => $services[$service]['name'],
  );
  $form['service'] = array(
    '#type' => 'machine_name',
    '#title' => t('Service machine name'),
    '#default_value' => $service,
    '#disabled' => TRUE,
    '#machine_name' => array(
      'source' => array('service_name'),
    ),
  );

  $form['service_icon'] = array(
    '#type' => 'textfield',
    '#title' => t('CSS classes'),
    '#description' => t('Separate multiple classes with spaces: <em>icon-facebook fa fa-facebook-square</em>'),
    '#default_value' => $services[$service]['icon'],
  );
  $form['service_validation_pattern'] = array(
    '#type' => 'textarea',
    '#title' => t('URL validation pattern'),
    '#description' => t('Enter the list of allowed URL patterns separated by new lines.<br />Leave empty to allow user input any URLs.<br />The "*" character is a wildcard.<br />Example: *facebook.com/* for any page on Facebook site.'),
    '#default_value' => $services[$service]['validation_pattern'],
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * Form submit for editing a service.
 */
function socialfield_edit_service_submit($form, $form_state) {
  $values = &$form_state['values'];
  $services = config_get('socialfield.settings', 'services');

  $service = array(
    'name' => $values['service_name'],
    'icon' => $values['service_icon'],
    'validation_pattern' => $values['service_validation_pattern'],
  );

  $services[$values['service']] = $service;

  config_set('socialfield.settings', 'services', $services);
  backdrop_goto('admin/config/media/socialfield');
}


/**
 * Form for deleting a service.
 */
function socialfield_delete_service($form, $form_state, $service = NULL) {
  if (!isset($service)) {
    backdrop_set_message(t('No service specified.'), 'error');
    backdrop_goto('admin/config/media/socialfield');
  }

  $services = config_get('socialfield.settings', 'services');

  if (!isset($services[$service])) {
    backdrop_set_message(t('The specified service does not exist.'), 'error');
    backdrop_goto('admin/config/media/socialfield');
  }

  $form['service'] = array(
    '#type' => 'hidden',
    '#value' => $service,
  );

  $question = t('Are you sure you want to delete @service service?', array('@service' => $service));
  $path = 'admin/config/media/socialfield';
  return confirm_form($form, $question, $path);
}

/**
 * Form submit for deleting a service.
 */
function socialfield_delete_service_submit($form, $form_state) {
  $values = &$form_state['values'];
  $services = config_get('socialfield.settings', 'services');

  unset($services[$values['service']]);

  config_set('socialfield.settings', 'services', $services);
  backdrop_set_message(t('@service service was successfully deleted.', array('@service' => $values['service'])));
  backdrop_goto('admin/config/media/socialfield');
}
