<? 
if (null == ($order = tpl_sort_order('issue')))
	$order = array('TS_RESOLVE_BY');
$iterator = new MethodFilterIterator($node->getChildren()->getIterator(0, null, $order), 'isPermitted', 'read');
$iterator->rewind();

if ($iterator->valid()) { ?>
<tr>
	<td>
		<? $this->render($node, 'table.tpl', array('list'=>$iterator,'partial'=>'subissues','request'=>$request,'path'=>'issue/'.$node->objectId().'/')) ?>
	</td>
</tr>
<? } ?>
