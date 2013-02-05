<?php
/**
 * @file
 * @copyright The Royal National Theatre
 * @author John-Paul Drawneek <jdrawneek@nationaltheatre.org.uk>
 */

class NtFeedTumblr extends NtFeedData {
  
  protected $total;
  protected $source = 'http://|BLOG|.tumblr.com/api/read/json';
  protected $url;

  public static function fixJson($json) {
    if (strpos($json, 'tumblr_api_read')) {
      $json = str_replace('var tumblr_api_read = ', '', trim($json));
      $json = substr($json, 0, strlen($json)-1);
    }
    return $json;
  }

  public static function getDescription($item) {
    switch ($item->type) {
      case 'link':
        $type = "link-description";
        break;
      case 'regular':
        $type = "regular-title";
        break;
      default:
        $type = $item->type . '-caption';
        break;
    }
    return $item->$type;
  }

  public function __construct($transport, $options) {
    $this->transport = $transport;
    $this->options = $options;
    $this->label = trim($options['screen_name']);
    $this->title = 'Tumblr - ' . trim($options['screen_name']);
    $this->id = md5(
      __CLASS__ . trim($options['screen_name'])
    );
    $this->url = str_replace('|BLOG|', $options['screen_name'], $this->source);
    $this->getItems($this->url);
  }

  public function filter($filter) {
    
  }

  public function paginate($number = 10) {
    
  }

  public function valid() {
    if (!isset($this->data[$this->pointer])) {
      $url = $this->url;
      if (count($this->data) == 0) {
        $url = $this->url;
      }
      elseif (count($this->data) < $this->total) {
        $url = $this->url . '?start=' . count($this->data);
      }
      else {
        return FALSE;
      }
      $this->getItems($url);
      if (!isset($this->data[$this->pointer])) {
        return FALSE;
      }
    }
    return TRUE;
  }
  
  protected function getItems($url) {
    $result = $this->transport->get(
      array(
        'url' => $url,
        'options' => array(
          'method' => 'GET',
        ),
      )
    );
    if ($result->code == 200) {
      $result->data = self::fixJson($result->data);
      $data = json_decode($result->data);
      $this->total = $data->{"posts-total"};
      if (is_object($data) && isset($data->posts)) {
        $data = $data->posts;
      }
      foreach ($data as $item) {
        if ($item->id == $this->lastPointer) {
          continue;
        }
         $store = array(
          'raw_data' => $item,
          'title' => $item->type,
          'date' => $item->{'unix-timestamp'},
          'id' => $item->id,
          'html_id' => 'tweet_' . $item->id,
          'description' => drupal_substr(self::getDescription($item), 0, 50),
          'url' => $item->{'url-with-slug'},
        );
         $this->dataKey[$item->id] = $store;
         $this->data[] = $this->dataKey[$item->id];
      }
      if (is_object($this->data[count($this->data)-1])) {
        $this->lastPointer = $this->data[count($this->data)-1]->id;
      }
      else {
        $this->lastPointer = $this->data[count($this->data)-1]['id'];
      }
      //die('<pre>' . print_r($result, TRUE) . '</pre>');
      return count($data) > 0;
    }
    else {
      // Log error message.
    }
    //die('<pre>' . print_r($result, TRUE) . '</pre>');
    return FALSE;
  }
  
  public function render($id) {
    $output = parent::render($id);
    $output['data']['type']['#value'] = $this->getId();
    $output['data']['feed_type']['#value'] = 'Tumblr';
    $output['data']['label']['#markup'] = 'Tumblr - ' . t($this->dataKey[$id]['description']);
    $output['data']['del']['#name'] = __CLASS__ . '_' . $id . '_del';
    $output['data']['del']['#data']['type'] = $this->getId();
    return $output;
  }

  public function getItem($id) {
    $url = $this->url . '?id=' . $id;
    $result = $this->transport->get(
      array(
        'url' => $url,
        'options' => array(
          'method' => 'GET',
        ),
      )
    );
    if ($result->code == 200) {
      $result->data = self::fixJson($result->data);
      $data = json_decode($result->data);
      $store = array(
        'raw_data' => $data->tumblelog,
        'title' => $data->tumblelog->type,
        'date' => $data->tumblelog->{'unix-timestamp'},
        'id' => $data->tumblelog->id,
        'html_id' => 'tweet_' . $data->tumblelog->id,
        'description' => drupal_substr(self::getDescription($data->tumblelog), 0, 50),
        'url' => $data->{'url-with-slug'},
      );
      $this->dataKey[$data->tumblelog->id] = $store;
    }
  }
}
