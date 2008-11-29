<?php

//echo '<pre>', print_r($_REQUEST),'</pre>';

require_once('../mudadmin/smf/SSI.php');
require_once('../mudadmin/smf/Sources/Subs-Post.php');

//Sanitize/organize some form input

switch ($_REQUEST['feedback_category']) {
  case "general":
    $form_category = "GENERAL";
    break;
  case "help":
    $form_category = "HELP";
    break;
  case "idea":
    $form_category = "IDEA";
    break;
  case "bug":
    $form_category = "BUG";
    break;
  case "complaint":
    $form_category = "COMPLAINT";
    break;
  case "compliment":
    $form_category = "COMPLIMENT";
    break;
  case "volunteer":
    $form_category = "VOLUNTEER";
    break;
}

if($_REQUEST['existing_player'] == 'yes')
  $form_existing_player = 'Yes';
else
  $form_existing_player = 'No';

$form_contact_methods = '';
if ( array_key_exists( 'contact_by_email', $_REQUEST) )
  if ( $_REQUEST['contact_by_email'] == 'yes' )
    $form_contact_methods .= ' email';
if ( array_key_exists( 'contact_by_mudmail', $_REQUEST) )
  if ( $_REQUEST['contact_by_mudmail'] == 'yes' )
    $form_contact_methods .= ' mudmail';
if ( array_key_exists( 'contact_by_in-mud', $_REQUEST) )
  if ( $_REQUEST['contact_by_in-mud'] == 'yes' )
    $form_contact_methods .= ' in-mud';
if ($form_contact_methods == '')
  $form_contact_methods = 'none';

$form_account_name = $_REQUEST['account_name'];
$form_real_name = $_REQUEST['real_name'];
$form_contact_info = $_REQUEST['contact_info'];
$form_subject = substr($_REQUEST['subject'], 0, 60);
$form_body = $_REQUEST['body'];

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! //
//  Message formatting heredoc starts here  //
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! //

$message_body = <<<EOD
Existing player?: $form_existing_player
Account name: $form_account_name
Real name: $form_real_name
Preferred contact method(s): $form_contact_methods
Contact info: $form_contact_info

Subject: $form_subject
Message:
[quote] $form_body [/quote]
EOD;

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! //
//  Message formatting heredoc ends here  //
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! //


// Setup the variables.
$msgOptions = array(
	'subject' => mysql_real_escape_string($form_category . ": " . $form_subject),
	'body' => mysql_real_escape_string($message_body),
	'id' => 0,
);

$topicOptions = array(
	'board' => 11,
	'id' => 0,
	'mark_as_read' => true,
);

$posterOptions = array(
	'email' => 'mudadmin@sneezymud.com',
	'ip' => '127.0.0.1',
	'name' => 'webform',
	'update_post_count' => 1,
	'id' => 80,
);

createPost($msgOptions, $topicOptions, $posterOptions);

// fuck php
//http_redirect("http://www.sneezymud.com/Thanks_for_your_feedback.html");

header("Location: http://www.sneezymud.com/Thanks_for_your_feedback.html");
echo "\n";

?>

Thanks for you feedback!
