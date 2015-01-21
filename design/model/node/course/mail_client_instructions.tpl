<?= tpl_text('Account for %s', $node->toString()) ?> 

<?= tpl_text('You can login to your account using the following address') ?> 
  <?= tpl_request_host() ?><?= tpl_view('course') ?> 
 
<?= tpl_text('Your account details') ?> 
  <?= tpl_text('Username: %s', $user->getLogin()) ?> 
  <?= tpl_text('Password: %s', $password) ?> 