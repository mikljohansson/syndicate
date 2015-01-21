<?
require_once 'design/gui/ISpreadsheetBuilder.class.inc';
$builder = new HtmlSpreadsheetBuilder();

require 'service.inc';
print $builder->toString();
