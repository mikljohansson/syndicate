function Issue() {}

/**
 * Creates a new click handler for a given menu option
 * @param	string		URI of XML-RPC module
 * @param	object		The row to build menu for
 * @param	array		The option spec from the server
 * @param	array		The selected ids
 * @return	function
 */
Issue.newActionHandler = function(uri, row, option, selection) {
	return function() {
		Issue._menu._hide();
		Issue._menu = null;

		// Find partial refresh capable block
		var block = null, blockuri = null;
		for (block = row.parentNode; block; block = block.parentNode) {
			if (block.getAttribute && (blockuri = block.getAttribute('partial')))
				break;
		}
		
		if (blockuri) {
			var request = {
				'toggle':		selection.length > 1 ? 0 : 1,
				'selection':		selection};
			request[option[2]] = option[3];
			
			var uricomp = blockuri.split('?', 2);
			var callspecs = [
				['', 'request', [option[1], request]],
				['', 'request', [uricomp[0], uricomp[1]]]
			];
			
			new JsonTransport(uri).invoke('multicall', [callspecs], function(result) {
				block.innerHTML = result[1];
			});

			for (var i=0; i<selection.length; i++) {
				if (null != (row = document.getElementById(selection[i]))) {
					for (var j=0, checkboxes = row.getElementsByTagName('input'); j<checkboxes.length; j++)
						checkboxes.item(j).checked = false;
					row.setAttribute('_checked', 'false');
				}
			}
		}
		else {
			var form = document.createElement('form');
			form.action = option[1] + (-1==option[1].indexOf('?')?'?':'&') + 'stack[]=' + encodeURIComponent(window.location);
			form.method = 'post';
			form.style.display = 'none';

			input = document.createElement('input');
			input.type = 'hidden';
			input.name = option[2];
			input.value = option[3];
			form.appendChild(input);

			input = document.createElement('input');
			input.type = 'hidden';
			input.name = 'collections[]';
			input.value = 'type.ole_collection.selection';
			form.appendChild(input);

			if (selection.length <= 1) {
				input = document.createElement('input');
				input.type = 'hidden';
				input.name = 'toggle';
				input.value = '1';
				form.appendChild(input);
			}

			for (var i=0; i<selection.length; i++) {
				input = document.createElement('input');
				input.type = 'hidden';
				input.name = 'selection[]';
				input.value = selection[i];
				form.appendChild(input);
			}

			document.getElementsByTagName('body').item(0).appendChild(form).submit();
		}
	};
}

/**
 * Creates a new context menu
 * @param	ContextMenu	Menu to populate
 * @param	string		URI of XML-RPC module
 * @param	object		The row to build menu for
 * @param	array		Array of options
 * @param	array		The selected ids
 * @return	ContextMenu
 */
Issue.buildContextMenu = function(menu, uri, row, options, selection) {
	if (options.length) {
		for (var i=0; i<options.length; i++) {
			action = null;
			if (options[i][3]) 
				action = Issue.newActionHandler(uri, row, options[i], selection);
			
			if (!options[i][4])
				menu.appendChild(new ContextItem(options[i][0], false, action));
			else {
				var item = new ContextItem(options[i][0], true, action);
				item.setFactory(Issue.newRemoteFactory(uri, row, options[i][4][0], options[i][4][1], options[i][4][2], selection));
				menu.appendChild(item, options[i][4][0]);
			}
		}
	}
	else {
		menu.appendChild(new ContextItem('(No options)'));
	}
	return menu;
}

/**
 * Creates a new menu factory
 *
 * The returned factory is capable of creating a new ContextMenu from
 * the given options.
 *
 * @param	string		URI of XML-RPC module
 * @param	object		The row to build menu for
 * @param	object		Associative array of option specs from server
 * @param	array		Ids of selected rows
 * @return	function
 */
Issue.newOptionsFactory = function(uri, row, options, selection) {
	return function(menu, onreadycallback) {
		onreadycallback(Issue.buildContextMenu(menu, uri, row, options, selection));
	};
}

/**
 * Creates a new menu factory
 *
 * The returned factory is capable of fetching menu options from 
 * server and creating a new ContextMenu
 *
 * @param	string		URI of XML-RPC module
 * @param	object		The row to build menu for
 * @param	string		Endpoint to fetch menu options from
 * @param	string		Method to invoke to fetch options
 * @param	array		Arguments to method
 * @param	array		Ids of selected rows
 * @return	function
 */
Issue.newRemoteFactory = function(uri, row, endpoint, method, args, selection) {
	return function(menu, onreadycallback) {
		new JsonTransport(endpoint).invoke(method, args, function(options) {
			onreadycallback(Issue.buildContextMenu(menu, uri, row, options, selection));
		});
	};
}

Issue.loadProject = function(base, selectbox, target) {
	try {
		var transport = new JsonTransport(selectbox.value.length ? base + 'node.' + selectbox.value + '/' : base + 'issue/');
		transport.invoke('getAssignedOptions', new Array(), function(options) {Issue._populate(target, options);});
	}
	catch (e) {
		if (!(e instanceof SyndUnsupportedException))
			throw e;
	}
}

Issue._populate = function(selectbox, options) {
	var child, selected = null;
	if (-1 != selectbox.selectedIndex)
		selected = selectbox[selectbox.selectedIndex];

	// Remove previous options
	for (var current = selectbox.firstChild, next = undefined; current; current = next) {
		next = current.nextSibling;
		if (undefined == current.className || 'Predefined' != current.className)
			selectbox.removeChild(current);
	}

	// Populate selectbox with new options
	if ('object' == typeof options) {
		for (id in options) {
			if ('string' == typeof options[id]) {
				child = selectbox.appendChild(document.createElement('option'));
				child.value = id;

				child.innerText = options[id];		// IE
				child.textContent = options[id];	// Mozilla

				if (null != selected && id == selected.value)
					child.selected = 'selected';
			}
		}
	}

	// Prepend previously selected option
	if (null != selected && undefined == options[selected.value])
		selectbox.insertBefore(selected, selectbox.firstChild);
}

Issue._match = function(value, prefix) {
	for (var i=0; i<prefix.length; i++) {
		if (prefix.charAt(i).toLowerCase() != value.charAt(i).toLowerCase())
			return false;
	}
	return true;
}

Issue.findOption = function(selectbox, ev) {
	var code = ev.charCode ? ev.charCode : ev.keyCode;
	if (!ev.altKey && !ev.ctrlKey && !ev.metaKey && code >= 32) {
		var ts = new Date().getTime(), prefix;
		if (selectbox._keyts && selectbox._keyts > ts - 500)
			prefix = selectbox._keyprefix + String.fromCharCode(code);
		else
			prefix = String.fromCharCode(code);
		
		var end = selectbox.selectedIndex ? selectbox.selectedIndex-1 : selectbox.options.length-1;
		for (var i = selectbox.selectedIndex; i != end; i = (i >= selectbox.options.length - 1) ? 0 : i+1) {
			if (Issue._match(selectbox.options[i].title, prefix)) {
				selectbox.selectedIndex = i;
				break;
			}
		}
		
		selectbox._keyts = ts;
		selectbox._keyprefix = prefix;
		
		ev.cancelBubble = true;
		return false;
	}
}

Issue.toggleQuote = function(node, block, showMessage, hideMessage) {
	if ('block' == block.style.display) {
		node.innerText = showMessage;
		block.style.display = 'none';
	} else {
		node.innerText = hideMessage;
		block.style.display = 'block';
	}
}

function Issues() {}

Issues.show = function(node, ids) {
	node.parentNode.setAttribute('_checked', 'true');

	if (null != ids) {
		for (var i=0, row; i<ids.length; i++) {
			if (null != (row = document.getElementById('node.' + ids[i])))
				row.setAttribute('_highlighted', 'true');
		}
		node._highlighted = ids;
	}
}	

Issues.hide = function(node) {
	node.parentNode.setAttribute('_checked', node.firstChild.checked ? 'true' : 'false');

	if (node._highlighted) {
		for (var i=0, row; i<node._highlighted.length; i++) {
			if (null != (row = document.getElementById('node.' + node._highlighted[i])))
				row.setAttribute('_highlighted', 'false');
		}
	}
}

Issues.select = function(input) {
	var nodes = input.parentNode.parentNode.parentNode.parentNode.getElementsByTagName('input'), i;
	for (i=0; i<nodes.length; i++) {
		if ('checkbox' == nodes.item(i).type.toLowerCase()) {
			nodes.item(i).checked = input.checked;
			nodes.item(i).parentNode.parentNode.setAttribute('_checked', input.checked ? 'true' : 'false');
		}
	}
}

function debug(obj) {
	var s = '';
	for (p in obj) {
		if ('function' != typeof obj[p])
			s += p+":"+obj[p]+"      ";
	}
	alert('' == s ? obj : s);
}
