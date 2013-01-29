<?php

/**
 * @file
 * Post editing UI.
 */

/**
 * UI controller.
 */
class PostUIController extends EntityDefaultUIController {

  /**
   * Overrides hook_menu() defaults. Main reason for doing this is that
   * parent class hook_menu() is optimized for entity type administration.
   */
  public function hook_menu() {
    watchdog('post_ui_controller', '<pre>' . print_r($this, TRUE) . '</pre>');
    $items = array();
    $id_count = count(explode('/', $this->path));
    $wildcard = isset($this->entityInfo['admin ui']['menu wildcard']) ? $this->entityInfo['admin ui']['menu wildcard'] : '%' . $this->entityType;

    $items[$this->path] = array(
      'title' => 'Posts',
      'description' => 'Add edit and update posts.',
      'page callback' => 'system_admin_menu_block_page',
      'access arguments' => array('access administration pages'),
      'file path' => drupal_get_path('module', 'system'),
      'file' => 'system.admin.inc',
    );
    
    // Change the overview menu type for the list of posts.
    $items[$this->path]['type'] = MENU_LOCAL_TASK;
    
    // Change the add page menu to multiple types of entities
    $items[$this->path . '/add'] = array(
      'title' => 'Add a post',
      'description' => 'Add a new post',
      'page callback'  => 'post_add_page',
      'access callback'  => 'post_access',
      'access arguments' => array('edit'),
      'type' => MENU_NORMAL_ITEM,
      'weight' => 20,
      'file' => 'post.admin.inc',
      'file path' => drupal_get_path('module', $this->entityInfo['module'])
    );
    
    // Add menu items to add each different type of entity.
    foreach (post_get_types() as $type) {
      $items[$this->path . '/add/' . $type->type] = array(
        'title' => 'Add ' . $type->label,
        'page callback' => 'post_form_wrapper',
        'page arguments' => array(post_create(array('type' => $type->type))),
        'access callback' => 'post_access',
        'access arguments' => array('edit', 'edit ' . $type->type),
        'file' => 'post.admin.inc',
        'file path' => drupal_get_path('module', $this->entityInfo['module'])
      );

      // AJAX setup.
      $items['post/nojs/add/'.$type->type] = array(
        'page arguments' => array(post_create(array('type' => $type->type)), 1),
      ) + $items[$this->path . '/add/' . $type->type];

      $items['post/ajax/add/'.$type->type] = array(
        'page arguments' => array(post_create(array('type' => $type->type)), 1),
        'delivery callback' => 'ajax_deliver',
      ) + $items[$this->path . '/add/' . $type->type];
    }

    // Loading and editing post entities
    $items[$this->path . '/post/' . $wildcard] = array(
      'page callback' => 'post_form_wrapper',
      'page arguments' => array($id_count + 1),
      'access callback' => 'post_access',
      'access arguments' => array('edit', $id_count + 1),
      'weight' => 0,
      'context' => MENU_CONTEXT_PAGE | MENU_CONTEXT_INLINE,
      'file' => 'post.admin.inc',
      'file path' => drupal_get_path('module', $this->entityInfo['module'])
    );
    $items[$this->path . '/post/' . $wildcard . '/edit'] = array(
      'title' => 'Edit',
      'type' => MENU_DEFAULT_LOCAL_TASK,
      'weight' => -10,
      'context' => MENU_CONTEXT_PAGE | MENU_CONTEXT_INLINE,
    );
    
    $items[$this->path . '/post/' . $wildcard . '/delete'] = array(
      'title' => 'Delete',
      'page callback' => 'post_delete_form_wrapper',
      'page arguments' => array($id_count + 1),
      'access callback' => 'post_access',
      'access arguments' => array('edit', $id_count + 1),
      'type' => MENU_LOCAL_TASK,
      'context' => MENU_CONTEXT_INLINE,
      'weight' => 10,
      'file' => 'post.admin.inc',
      'file path' => drupal_get_path('module', $this->entityInfo['module'])
    );
    
    // Menu item for viewing posts
    $items['post/' . $wildcard] = array(
      //'title' => 'Title',
      'title callback' => 'post_page_title',
      'title arguments' => array(1),
      'page callback' => 'post_page_view',
      'page arguments' => array(1),
      'access callback' => 'post_access',
      'access arguments' => array('view', 1),
      'type' => MENU_CALLBACK,
    );
    return $items;
  }
  
  
  /**
   * Create the markup for the add Post Entities page within the class
   * so it can easily be extended/overriden.
   */ 
  public function addPage() {
    $item = menu_get_item();
    $content = system_admin_menu_block($item);

    if (count($content) == 1) {
      $item = array_shift($content);
      drupal_goto($item['href']);
    }    
        
    return theme('post_add_list', array('content' => $content));
  }
  
}


/**
 * Form callback wrapper: create or edit a post.
 *
 * @param $post
 *   The post object being edited by this form.
 * @param $callback
 *   Essentially the callback type, whether it is 'nojs' or 'ajax'
 *
 * @see post_edit_form()
 */
function post_form_wrapper($post, $callback) {
  // Add the breadcrumb for the form's location.
  post_set_breadcrumb();
  $post->callback_type = $callback;
  if ($callback == 'nojs') {
    return drupal_get_form('post_edit_form', $post);
  }
  else {
    // Just some ajax.
    // Need a workaround inorder to add dom id to the parameter.
    $commands = array();
    $form = drupal_render(drupal_get_form('post_edit_form', $post));
    // Find a way to dynamically point DOM elemenet
    $commands[] = ajax_command_html('#messages', $form);
    return array('#type' => 'ajax', '#commands' => $commands);
  }
}


/**
 * Form callback wrapper: delete a post.
 *
 * @param $post
 *   The post object being edited by this form.
 *
 * @see post_edit_form()
 */
function post_delete_form_wrapper($post) {
  // Add the breadcrumb for the form's location.
  //post_set_breadcrumb();
  return drupal_get_form('post_delete_form', $post);
}


/**
 * Form callback: create or edit a post.
 *
 * @param $post
 *   The post object to edit or for a create form an empty post object
 *     with only a post type defined.
 */
function post_edit_form($form, &$form_state, $post) {
  // Add the default field elements.
  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Post Name'),
    '#default_value' => isset($post->name) ? $post->name : '',
    '#maxlength' => 255,
    '#required' => TRUE,
    '#weight' => -5,
  );
  
  $form['data']['#tree'] = TRUE;
  $form['data']['sample_data'] = array(
    '#type' => 'checkbox',
    '#title' => t('An interesting post switch'),
    '#default_value' => isset($post->data['sample_data']) ? $post->data['sample_data'] : 1,
  );

  // Add the field related form elements.
  $form_state['post'] = $post;
  field_attach_form('post', $post, $form, $form_state);

  $form['actions'] = array(
    '#type' => 'container',
    '#attributes' => array('class' => array('form-actions')),
    '#weight' => 400,
  );

  // We add the form's #submit array to this button along with the actual submit
  // handler to preserve any submit handlers added by a form callback_wrapper.
  $submit = array();

  if (!empty($form['#submit'])) {
    $submit += $form['#submit'];
  }

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save post'),
    '#submit' => $submit + array('post_edit_form_submit'),
  );
  
  if (!empty($post->name)) {
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete post'),
      '#suffix' => l(t('Cancel'), 'admin/content/posts'),
      '#submit' => $submit + array('post_form_submit_delete'),
      '#weight' => 45,
    );
  }

  // We append the validate handler to #validate in case a form callback_wrapper
  // is used to add validate handlers earlier.
  $form['#validate'][] = 'post_edit_form_validate';
  return $form;
}


/**
 * Form API validate callback for the post form
 */
function post_edit_form_validate(&$form, &$form_state) {
  $post = $form_state['post'];
  
  // Notify field widgets to validate their data.
  field_attach_form_validate('post', $post, $form, $form_state);
}


/**
 * Form API submit callback for the post form.
 * 
 * @todo remove hard-coded link
 */
function post_edit_form_submit(&$form, &$form_state) {
  
  $post = entity_ui_controller('post')->entityFormSubmitBuildEntity($form, $form_state);
  // Save the post and go back to the list of posts
  // Add in created and changed times.
  if ($post->is_new = isset($post->is_new) ? $post->is_new : 0){
    $post->created = time();
  }

  $post->changed = time();
  
  $post->save();
  $form_state['redirect'] = 'admin/content/posts';
}

/**
 * Form API submit callback for the delete button.
 * 
 * @todo Remove hard-coded path
 */
function post_form_submit_delete(&$form, &$form_state) {
  $form_state['redirect'] = 'admin/content/posts/post/' . $form_state['post']->post_id . '/delete';
}


/**
 * Form callback: confirmation form for deleting a post.
 *
 * @param $post
 *   The post to delete
 *
 * @see confirm_form()
 */
function post_delete_form($form, &$form_state, $post) {
  $form_state['post'] = $post;

  $form['#submit'][] = 'post_delete_form_submit';

  $form = confirm_form($form,
    t('Are you sure you want to delete post %name?', array('%name' => $post->name)),
    'admin/content/posts/post',
    '<p>' . t('This action cannot be undone.') . '</p>',
    t('Delete'),
    t('Cancel'),
    'confirm'
  );
  
  return $form;
}

/**
 * Submit callback for post_delete_form
 */
function post_delete_form_submit($form, &$form_state) {
  $post = $form_state['post'];

  post_delete($post);

  drupal_set_message(t('The post %name has been deleted.', array('%name' => $post->name)));
  watchdog('post', 'Deleted post %name.', array('%name' => $post->name));

  $form_state['redirect'] = 'admin/content/posts';
}



/**
 * Page to add Post Entities.
 *
 * @todo Pass this through a proper theme function
 */
function post_add_page() {
  $controller = entity_ui_controller('post');
  return $controller->addPage();
}


/**
 * Displays the list of available post types for post creation.
 *
 * @ingroup themeable
 */
function theme_post_add_list($variables) {
  $content = $variables['content'];
  $output = '';
  if ($content) {
    $output = '<dl class="post-type-list">';
    foreach ($content as $item) {
      $output .= '<dt>' . l($item['title'], $item['href']) . '</dt>';
      $output .= '<dd>' . filter_xss_admin($item['description']) . '</dd>';
    }
    $output .= '</dl>';
  }
  else {
    if (user_access('administer post types')) {
      $output = '<p>' . t('Post Entities cannot be added because you have not created any post types yet. Go to the <a href="@create-post-type">post type creation page</a> to add a new post type.', array('@create-post-type' => url('admin/structure/post_types/add'))) . '</p>';
    }
    else {
      $output = '<p>' . t('No post types have been created yet for you to use.') . '</p>';
    }
  }

  return $output;
}





/**
 * Sets the breadcrumb for administrative post pages.
 */
function post_set_breadcrumb() {
  $breadcrumb = array(
    l(t('Home'), '<front>'),
    l(t('Administration'), 'admin'),
    l(t('Content'), 'admin/content'),
    l(t('Posts'), 'admin/content/posts'),
  );

  drupal_set_breadcrumb($breadcrumb);
}



