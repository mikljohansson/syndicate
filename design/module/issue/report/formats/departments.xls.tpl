<?
require_once 'design/gui/ISpreadsheetBuilder.class.inc';
$builder = new ExcelSpreadsheetBuilder();

require 'departments.inc';
print $builder->toString();