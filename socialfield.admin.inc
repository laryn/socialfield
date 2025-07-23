<?php
/**
 * @file
 * All code for administrative pages, left here for performance reasons.
 */

/**
 * Page callback: Table that displays all available services.
 */
function socialfield_settings($form, &$form_state) {
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
    '#description' => t('Change this setting to review the icons below.'),
    '#default_value' => $config->get('filled'),
  );

  $form['settings']['dont'] = array(
    '#type' => 'fieldset',
    '#title' => t('Not Recommended'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['settings']['dont']['new_windows'] = array(
    '#type' => 'checkbox',
    '#title' => t('Open links in new windows'),
    '#description' => t('Forcing links to open in new windows makes it difficult for people to find their way back to this site, and is generally bad for accessibility. <br>Enabling this setting could hinder SEO.'),
    '#default_value'  => $config->get('new_windows'),
  );
  $form['settings']['dont']['remove_hidden_text'] = array(
    '#type' => 'checkbox',
    '#title' => t('Remove hidden labels from social links  (not recommended)'),
    '#description' => t('Including the service name as hidden text is helpful for screen readers. <br>Enabling this setting could hinder SEO.'),
    '#default_value'  => $config->get('remove_hidden_text'),
  );

  $header = array(
    t('Service'),
    t('Icon'),
    t('Icon API Names'),
    t('Additional classes'),
    t('Validation pattern'),
    t('Operations'),
  );

  $rows = array();
  $services = $config->get('services');
  if (is_array($services)) {
    // Move the disabled services to the end.
    $disabled_services = array();
    foreach ($services as $name => $service_info) {
      if (!isset($service_info['enabled']) || $service_info['enabled'] == FALSE) {
        $disabled_services[$name] = $service_info;
        unset($services[$name]);
      }
    }
    $services = array_merge($services, $disabled_services);
    foreach ($services as $name => $service_info) {
      // Sanitize
      $machine_name = $service_info['service_name'];
      $service_name = check_plain($service_info['label']);
      $icon_classes_saved = check_plain($service_info['icon_css']);
      $validation_pattern = check_plain($service_info['validation_pattern']);
      $enabled = $service_info['enabled'];

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

      if ($enabled) {
        $dropbuttons['#links']['disable'] = array(
          'title' => t('Disable'),
          'href' => 'admin/config/media/socialfield/' . $machine_name . '/disable',
          'query' => array('token' => backdrop_get_token('disable-' . $machine_name)),
        );
      }
      else {
        $dropbuttons['#links']['enable'] = array(
          'title' => t('Enable'),
          'href' => 'admin/config/media/socialfield/' . $machine_name . '/enable',
          'query' => array('token' => backdrop_get_token('enable-' . $machine_name)),
        );
      }

      $iconapi_name = check_plain($service_info['iconapi']);
      $iconapi_fill_name = check_plain($service_info['iconapi_fill']);
      $icon_name = $iconapi_name;
      if ($config->get('filled')) {
        $icon_name = $iconapi_fill_name;
      }

      // The 'icon' classes is added by icon(), so we don't ad it here.
      $icon_classes = explode(' ', $icon_classes_saved);
      $icon_classes[] = 'socialfield';
      $icon_classes[] = 'socialfield-' . $machine_name;
      $icon_options = array(
        'attributes' => array(
          'class' => $icon_classes,
          'width' => 30,
          'height' => 30,
        ),
      );

      $classes = array();
      if (!$service_info['enabled']){
        $classes[] = 'disabled';
      }

      $rows[] = array(
        'data' => array(
          $service_name,
          icon($icon_name, $icon_options),
          $iconapi_name . '<br>' . $iconapi_fill_name,
          $icon_classes_saved,
          str_replace("\n", '<br />', $validation_pattern),
          backdrop_render($dropbuttons),
        ),
        'class' => $classes,
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
 * Callback to enable a service
 */
function socialfield_service_enable($service) {
  $config = config('socialfield.settings');
  $services = $config->get('services');
  $service_info = $services[$service];
  if (!isset($_GET['token']) || !backdrop_valid_token($_GET['token'], 'enable-' . $service_info['service_name'])) {
    return MENU_ACCESS_DENIED;
  }

  $services[$service]['enabled'] = TRUE;
  $config->set('services', $services);
  $config->save();

  backdrop_set_message(t('The @label service has been enabled.', array('@label' => $service_info['label'])));
  backdrop_goto('admin/config/media/socialfield');
}

/**
 * Callback to disable a service
 */
function socialfield_service_disable($service) {
  $config = config('socialfield.settings');
  $services = $config->get('services');
  $service_info = $services[$service];
  if (!isset($_GET['token']) || !backdrop_valid_token($_GET['token'], 'disable-' . $service_info['service_name'])) {
    return MENU_ACCESS_DENIED;
  }

  $services[$service]['enabled'] = FALSE;
  $config->set('services', $services);
  $config->save();

  backdrop_set_message(t('The @label service has been disabled.', array('@label' => $service_info['label'])));
  backdrop_goto('admin/config/media/socialfield');
}

/**
 * Form for adding a new service.
 */
function socialfield_service_form($form, $form_state, $service_info = NULL) {
  if ($service_info == NULL) {
    // Set empty defaults.
    $service_info = array(
      'label' => '',
      'service_name' => '',
      'iconapi' => '',
      'iconapi_fill' => '',
      'icon_css' => '',
      'validation_pattern' => '',
    );
  }

  $form['label'] = array(
    '#type' => 'textfield',
    '#title' => t('Service'),
    '#default_value' => $service_info['label'],
  );
  $form['service_name'] = array(
    '#type' => 'machine_name',
    '#title' => t('Service machine name'),
    '#default_value' => $service_info['service_name'],
    '#disabled' => TRUE,
    '#machine_name' => array(
      'source' => array('label'),
    ),
  );
  $iconapi_description = t('See <a href="https://docs.backdropcms.org/documentation/icon-api">the Backdrop Icon API</a> or <a href="https://phosphoricons.com">Phosphor icon list</a>.');
  $form['iconapi'] = array(
    '#type' => 'textfield',
    '#title' => t('Name of Icon API icon'),
    '#default_value' => $service_info['iconapi'],
    '#description' => $iconapi_description,
  );
  $form['iconapi_fill'] = array(
    '#type' => 'textfield',
    '#title' => t('Name of Filled version of Icon API icon'),
    '#default_value' => $service_info['iconapi_fill'],
    '#description' => $iconapi_description,
  );
  $form['icon_css'] = array(
    '#type' => 'textfield',
    '#title' => t('Additional CSS classes'),
    '#description' => t('Separate multiple classes with spaces: <em>icon-facebook fa fa-facebook-square</em>'),
    '#default_value' => $service_info['icon_css'],
  );
  $form['validation_pattern'] = array(
    '#type' => 'textarea',
    '#title' => t('URL validation pattern'),
    '#description' => t('Enter the list of allowed URL patterns separated by new lines.<br />Leave empty to allow user input any URLs.<br />The "*" character is a wildcard.<br />Example: *facebook.com/* for any page on Facebook site.'),
    '#default_value' => $service_info['validation_pattern'],
  );

  $form['actions'] = array(
    '#type' => 'actions',
  );
  // We exclude the button so the text can be different.

  return $form;
}

/**
 * Form for adding a new service.
 */
function socialfield_add_service($form, $form_state) {
  $form = socialfield_service_form($form, $form_state);

  $form['actions']['submit'] = array(
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
  else {
    $service_info = $services[$service];
    $service_info['service_name'] = $service;
  }

  $form = socialfield_service_form($form, $form_state, $service_info);
  $form['actions']['submit'] = array(
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
