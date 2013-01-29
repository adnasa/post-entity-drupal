<?php

/**
 * The class used for post type entities
 */
class PostType extends Entity {
  
  public $type;
  public $label;
  
  public function __construct($values = array()) {
    parent::__construct($values, 'post_type');
  }
  
  public function isLocked() {
    return isset($this->status) && empty($this->is_new) && (($this->status & ENTITY_IN_CODE) || ($this->status & ENTITY_FIXED));
  }
}

/**
 * The Controller for Post entities
 */
class PostTypeController extends EntityAPIControllerExportable {
  public function __construct($entityType) {
    parent::__construct($entityType);
  }
  
   /**
   * Create a post type - we first set up the values that are specific
   * to our post type schema but then also go through the EntityAPIController
   * function.
   * 
   * @param $type
   *   The machine-readable type of the post.
   *
   * @return
   *   A post type object with all default fields initialized.
   */
  public function create(array $values = array()) {
    // Add values that are specific to our Post
    $values += array( 
      'id' => '',
      'is_new' => TRUE,
      'data' => '',
    );
    $post_type = parent::create($values);
    return $post_type;
  }

}
