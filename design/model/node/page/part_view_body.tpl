<table>
	<tr>
		<td>
			<? 
			if (!isset($request['revision']) || !$node->isPermitted('write')) {
				$body = $node->getBody();
				print $body->toString();
			}
			else {
				require_once 'core/lib/SyndHTML.class.inc';
				$revision = SyndNodeLib::getInstance($request['revision']);
				$node1 = $revision->getRevision();

				$prevRevision = $revision->getPrevious();
				if (!$prevRevision->isNull()) {
					$node2 = $prevRevision->getRevision();
					print SyndHTML::highlightDiff(
						$node2->data['INFO_BODY']->toString(),
						$node1->data['INFO_BODY']->toString());
				}
				else {
					print SyndHTML::highlightDiff('',
						$node1->data['INFO_BODY']->toString());
				}
			}
			?>
		</td>

		<? if ($node->isPermitted('write') && count($revisions = $node->getRevisions())) { ?>
		<td style="width:13em;">
			<div class="Block Revisions">
				<a href="<?= tpl_link($node->getHandler(),'view',$node->nodeId) ?>"><b><?= tpl_text('Revisions') ?></b></a>
				<div class="indent">
					<? foreach (array_keys($revisions) as $key) { ?>
						<? if ($revisions[$key]->nodeId == $revision->nodeId) { ?>
						<div class="highlight" style="padding:1px;">
						<? } else { ?>
						<div style="padding:2px;">
						<? } ?>
							<a href="<?= tpl_link($node->getHandler(),'view',$node->nodeId,
								array('revision'=>$revisions[$key]->nodeId)) ?>"><?= 
								date('Y-m-d H:i', $revisions[$key]->getCreateTime()) ?></a><br />
							<? $this->render($revisions[$key]->getCustomer(),'head_view.tpl') ?><br />
						</div>
					<? } ?>
				</div>
			</div>
		</td>
		<? } ?>
	</tr>
</table>