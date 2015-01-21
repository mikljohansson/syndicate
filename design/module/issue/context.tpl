<?
tpl_load_script(tpl_design_uri('js/json.js'));
tpl_load_script(tpl_design_uri('js/contextmenu.js'));
tpl_load_stylesheet(tpl_design_uri('js/contextmenu.css'));
tpl_load_script(tpl_design_uri('module/issue/issue.js'));
?>
<script type="text/javascript">
<!--
	function issue_context_menu(row, ev, uri, isWorkload) {
		if (Issue._menu)
			Issue._menu._hide();

		row.setAttribute('_checked', 'true');

		var menu = new ContextMenu();
		Issue._menu = menu;
		this._row = row;

		menu.appendChild(new ContextItem('<?= $this->text('Assigned') ?>', true), 'assigned');
		menu.appendChild(new ContextItem('<?= $this->text('Project') ?>', true), 'project');
		menu.appendChild(new ContextItem('<?= $this->text('Categories') ?>', true), 'categories');
		menu.appendChild(new ContextItem('<?= $this->text('Priority') ?>', true), 'priority');
		menu.appendChild(new ContextItem('<?= $this->text('Status') ?>', true), 'status');
		menu.appendChild(new ContextItem('<?= $this->text('Reschedule') ?>', true), 'reschedule');
		<?= SyndLib::runHook('issue_context', $this) ?>
		
		if (isWorkload) 
			menu.appendChild(new ContextItem('Estimate', true), 'estimate');

		// Build issue selection
		selection = new Array();
		for (var i=0; i<row.parentNode.childNodes.length; i++) {
			if (row.parentNode.childNodes.item(i).getAttribute && 
				row.parentNode.childNodes.item(i).getAttribute('_checked') == 'true')
				selection.push(row.parentNode.childNodes.item(i).id);
		}

		// Check for multiple selection
		if (selection.length > 1)
			menu._node.className += ' MultipleSelection';

		// Retrive context menus from server
		new JsonTransport(uri + row.id).invoke('getContextMenus', new Array(), function(result) {
			for (id in result) {
				if (undefined != menu._submenus[id] && menu._submenus[id].setFactory)
					menu._submenus[id].setFactory(Issue.newOptionsFactory(uri, row, result[id], selection));
			}
		});

		var html = document.getElementsByTagName('html').item(0);
		menu._hide = function() {
			menu.hide();
			
			if (!(checkboxes = row.getElementsByTagName('input')).length)
				row.setAttribute('_checked', 'false');
			else {
				for (var j=0; j<checkboxes.length; j++)
					row.setAttribute('_checked', checkboxes.item(j).checked ? 'true' : 'false');
			}

			html.detachEvent('onclick', menu._hide);
			html.detachEvent('oncontextmenu', menu._hide);
			html.detachEvent('onkeydown', menu._hide);
		}

		html.attachEvent('onclick', menu._hide);
		html.attachEvent('oncontextmenu', menu._hide);
		html.attachEvent('onkeydown', menu._hide);

		menu.show(ev.clientX + html.scrollLeft, ev.clientY + html.scrollTop);

		ev.cancelBubble = true;
		return false;
	}
//-->
</script>