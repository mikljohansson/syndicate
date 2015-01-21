<?
require_once 'design/gui/ISpreadsheetBuilder.class.inc';
$builder = new ExcelSpreadsheetBuilder();

require 'service.inc';
print $builder->toString();
