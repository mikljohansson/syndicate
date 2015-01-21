<ul>
	<li><?= tpl_text("Searching using the keywords AND/OR allows for finer control of the results. For example; searching for <em>'internet AND explorer OR firefox'</em> would return documents containing both of the words 'internet' and 'explorer' as well as documents containing the word 'firefox'.") ?></li>
	<li><?= tpl_text("Use the +/- modifiers to include or exclude words from searches. For example; searching for <em>'browser -firefox'</em> would return documents containing the word 'browser' but lacking the word 'firefox'.") ?></li>
	<li><?= tpl_text("Search for complete phrases by enclosing them in double quotes. A search for <em>\"managing gigabytes\"</em> would return only documents containing that specific phrase.") ?></li>
	<li><?= tpl_text("Words and expressions can be grouped using parentheses. For example <em>'browser AND (firefox OR lynx)'</em> would return documents containing the word 'browser' and either of the words 'firefox' or 'lynx'."); ?></li>
	<li><?= tpl_text("Proximity searches utilize the NEAR keyword. For example <em>'browser NEAR firefox'</em> matches documents containing these words within 10 words of each other.") ?></li>
</ul>
