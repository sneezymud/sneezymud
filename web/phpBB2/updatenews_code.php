#!/usr/local/bin/php -q

<?php

define('IN_PHPBB', true);
$phpbb_root_path = '/mud/web/phpBB2/';
include($phpbb_root_path . 'extension.inc');
include($phpbb_root_path . 'common.'.$phpEx);
include($phpbb_root_path . 'includes/functions_post.'.$phpEx);
include($phpbb_root_path . 'includes/bbcode.'.$phpEx);
include($phpbb_root_path . 'includes/functions_search.'.$phpEx);

$fd = fopen ('php://stdin', "r");
$message = fread ($fd, 16384);
fclose ($fd);

if($message==""){
  exit;
}

// forum 11 = code updates
insert_post("[code]".$message."[/code]", substr($message, 0, 45), 11, 80, "news", 0);





function insert_post($message, $subject, $forum_id, $user_id, $user_name, $user_attach_sig)
{
   global $db, $board_config;

   $post_subject = $subject;
   $post_message = $message;
   $html_on = $board_config['allow_html'];
   $bbcode_on = $board_config['allow_bbcode'];
   $smilies_on = $board_config['allow_smilies'];
   $attach_sig = $user_attach_sig;
   $topic_type = POST_NORMAL;
   $topic_vote = 0;
   $notify_user = 0;
   $poll_title = '';
   $poll_options = '';
   $poll_length = '';
   $post_username = $user_name;
   $bbcode_uid = make_bbcode_uid();
   $mode == 'newtopic';
   $current_time = time();

   $post_data = array();
   $post_data['poster_post'] = false;
   $post_data['first_post'] = false;
   $post_data['last_post'] = false;
   $post_data['last_topic'] = false;
   $post_data['has_poll'] = false;
   $post_data['topic_type'] = POST_NORMAL;
   $post_data['poster_id'] = $user_id;
   
   $post_message = prepare_message(trim($post_message), $html_on, $bbcode_on, $smilies_on, $bbcode_uid);
   $post_message = unprepare_message($post_message);
   $post_subject = str_replace("'", "''", $post_subject);
   $post_message = str_replace("'", "''", $post_message);
   $post_username = str_replace("'", "''", $post_username);
   if ( $error_msg == '' )
   {
      $sql = "INSERT INTO " . TOPICS_TABLE . " (topic_title, topic_poster, topic_time, forum_id, topic_status, topic_type, topic_vote) VALUES ('$post_subject', " . $user_id . ", $current_time, $forum_id, " . TOPIC_UNLOCKED . ", $topic_type, $topic_vote)";
      if ( !$db->sql_query($sql, BEGIN_TRANSACTION) )
      {
         message_die(GENERAL_ERROR, 'Error in posting 1', '', __LINE__, __FILE__, $sql);
         //exit;
      }
      $topic_id = $db->sql_nextid();
               
      $edited_sql = "";
      $sql = "INSERT INTO " . POSTS_TABLE . " (topic_id, forum_id, poster_id, post_username, post_time, poster_ip, enable_bbcode, enable_html, enable_smilies, enable_sig) VALUES ($topic_id, $forum_id, " . $user_id . ", '$post_username', $current_time, '$user_ip', $bbcode_on, $html_on, $smilies_on, $attach_sig)";
      if ( !$db->sql_query($sql) )
      {
         message_die(GENERAL_ERROR, "Error in posting 2 $sql", '', __LINE__, __FILE__, $sql);
         //exit;
      }
      $post_id = $db->sql_nextid();

      $sql = "INSERT INTO " . POSTS_TEXT_TABLE . " (post_id, post_subject, bbcode_uid, post_text) VALUES ($post_id, '$post_subject', '$bbcode_uid', '$post_message')";
      if ( !$db->sql_query($sql, END_TRANSACTION) )
      {
         message_die(GENERAL_ERROR, 'Error in posting 3', '', __LINE__, __FILE__, $sql);
         //exit;
      }
      
      $sql = "UPDATE " . FORUMS_TABLE . " SET
         forum_posts = forum_posts + 1,
         forum_last_post_id = $post_id,
         forum_topics = forum_topics + 1      
         WHERE forum_id = $forum_id";
      if ( !$db->sql_query($sql, BEGIN_TRANSACTION) )
      {
         message_die(GENERAL_ERROR, 'Error in posting 4', '', __LINE__, __FILE__, $sql);
         //exit;
      }

      $sql = "UPDATE " . TOPICS_TABLE . " SET
            topic_last_post_id = $post_id,
            topic_first_post_id = $post_id
            WHERE topic_id = $topic_id";
      if ( !$db->sql_query($sql) )
      {
         message_die(GENERAL_ERROR, 'Error in posting 5', '', __LINE__, __FILE__, $sql);
         //exit;
      }


      $sql = "UPDATE " . USERS_TABLE . "
         SET user_posts = user_posts + 1
         WHERE user_id = $user_id";
      if ( !$db->sql_query($sql, END_TRANSACTION) )
      {
         message_die(GENERAL_ERROR, 'Error in posting 6', '', __LINE__, __FILE__, $sql);
         //exit;
      }
      
      add_search_words($post_id, stripslashes($post_message), stripslashes($post_subject));
      return $topic_id;
   }
   else
   {
      $error_msg .= $error_msg . '<br>' . $error_msg;
      return $error_msg;
   }   
} 


?>
