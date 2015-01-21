<? 
$activities = $node->getActivity()->getActivities();
if (count($activities) == 1 && $this->path(get_class(SyndLib::reset($activities)), 'menuitem.tpl', false)) { ?>
	<? $this->render(SyndLib::reset($activities), 'menuitem.tpl', array('name' => $node->toString(), 'accesskey' => $node->data['INFO_ACCESSKEY'])) ?>
<? } else { ?>
	<? if (strlen($node->data['INFO_ACCESSKEY'])) { ?>
	<a href="javascript:synd_ole_call('<?= tpl_attribute(tpl_view_call('issue','batch',array('workflow'=>$node->id(),'post'=>0))) ?>')" accesskey="<?= $this->quote($node->data['INFO_ACCESSKEY']) ?>" title="<?= $this->text('Accesskey: %s',$node->data['INFO_ACCESSKEY']) ?>"><?= $this->quote($node->toString()) ?></a>
	<? } else { ?>
	<a href="javascript:synd_ole_call('<?= tpl_attribute(tpl_view_call('issue','batch',array('workflow'=>$node->id(),'post'=>0))) ?>')"><?= $this->quote($node->toString()) ?></a>
	<? } ?>
<? } ?>
