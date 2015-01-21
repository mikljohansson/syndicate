<?php

if ($node->data['FLAG_PINGBACKS']) {
	assert('!headers_sent()');
	header('X-Pingback: '.tpl_request_host().tpl_view('rpc','json',$node->id()));
}
