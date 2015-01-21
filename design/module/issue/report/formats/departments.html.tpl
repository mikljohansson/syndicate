<?
require_once 'design/gui/ISpreadsheetBuilder.class.inc';
$builder = new HtmlSpreadsheetBuilder();

require 'departments.inc';
print $builder->toString();