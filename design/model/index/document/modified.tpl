<?
			if (null != $node->getModificationTime()) {
				print tpl_translate('(%dKb, last modifed %s) - <a href="%s">View as HTML</a>', 
					$node->getSize()/1024, tpl_quote(date('Y-m-d', $node->getModificationTime())), 
					tpl_link('search','display',array('uri'=>$node->getLocation(),'query'=>$request['query'])));
			}
			else {
				print tpl_translate('(%dKb) - <a href="%s">View as HTML</a>', 
					$node->getSize()/1024, 
					tpl_link('search','display',array('uri'=>$node->getLocation(),'query'=>$request['query'])));
			}
