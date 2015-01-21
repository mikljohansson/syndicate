Course account for <?= $node->toString() ?> 

You can login to your account using the following address
  <?= tpl_request_host() ?><?= tpl_view('course') ?> 
 
Your account details
  Username: <?= $user->getLogin() ?> 