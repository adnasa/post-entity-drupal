<?php

/**
 * The class used for post entities
 */
class Post extends Entity {
  
  public function __construct($values = array()) {
    parent::__construct($values, 'post');
  }

  protected function defaultLabel() {
    return $this->name;
  }

  protected function defaultUri() {
    return array('path' => 'post/' . $this->post_id);
  }
  
  
}

/**
 * The Controller for Post entities
 */
class PostController extends EntityAPIController {
  public function __construct($entityType) {
    parent::__construct($entityType);
  }


  /**
   * Create a post - we first set up the values that are specific
   * to our post schema but then also go through the EntityAPIController
   * function.
   * 
   * @param $type
   *   The machine-readable type of the post.
   *
   * @return
   *   A post object with all default fields initialized.
   */
  public function create(array $values = array()) {
    // Add values that are specific to our Post
    $values += array( 
      'post_id' => '',
      'is_new' => TRUE,
      'title' => '',
      'created' => '',
      'changed' => '',
      'data' => '',
    );
    
    $post = parent::create($values);
    return $post;
  }
  
  /**
   * Overriding the buldContent function to add entity specific fields
   */
  public function buildContent($entity, $view_mode = 'full', $langcode = NULL, $content = array()) {
    $content = parent::buildContent($entity, $view_mode, $langcode, $content);
    $content['post_sample_data'] =  array(
      '#markup' => theme('post_sample_data', array('post_sample_data' => check_plain($entity->data['sample_data']), 'post' => $entity)),
    );

    return $content;
  }
  
}



