	<div class="EmailHeader">
		<? if (null != ($from = $message->getHeader('From'))) { ?>
		<?= tpl_translate('From: %s', tpl_email($from)) ?><br /><? } ?>
		<? if (null != ($to = $message->getHeader('To'))) { ?>
		<?= tpl_translate('To: %s', tpl_email($to)) ?><br /><? } ?>
		<? if (null != ($cc = $message->getHeader('Cc'))) { ?>
		<?= tpl_translate('Cc: %s', tpl_email($cc)) ?><br /><? } ?>
		<? if (null != ($subject = $message->getHeader('Subject'))) { ?>
		<?= tpl_text('Subject: %s', $subject) ?><br /><? } ?>
	</div>
	<?= SyndLib::runHook('email_render_html', $node->getParent(), $message) ?>
<? if (null !== $node->_content) { ?>
	<p>
		<?= $node->hideQuotedText($this, tpl_filter($filter, 
			$node->_content, 
			$message->getHeader('Content-Type', 'charset'))) ?>
	</p>
<? } else if (count($parts = $message->getParts()) && 'multipart/alternative' != strtolower($message->getHeader('Content-Type'))) { ?>
	<? foreach (array_keys($parts) as $key) { ?>
		<? $this->render($node,'render/html.tpl',array('message'=>$parts[$key],'filter'=>$filter)) ?>
	<? } ?>
<? } else if (null != $message->getContent()) { ?>
	<? if (!$message->isAttachment()) { ?>
		<p>
			<?= $node->hideQuotedText($this, tpl_filter($filter, 
				$message->getContent(), 
				$message->getHeader('Content-Type', 'charset'))) ?>
		</p>
	<? } ?>
<? } ?>
