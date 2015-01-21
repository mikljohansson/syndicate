<?php
require_once 'core/index/IndexBuilder.class.inc';

class synd_parser_ctype extends ForwardingIndexBuilder {
	function visitFragment($fragment, $field = null, $weight = 1) {
		for ($i=0,$j=0,$c=strlen($fragment); $i<$c; $i=$j) {
			while ($i<$c && !ctype_alnum($fragment[$i]))
				$i++;
			$j = $i+1;
			while ($j<$c && ctype_alnum($fragment[$j]))
				$j++;
			if ($i < $c)
				$this->_next->visitTerm(substr($fragment, $i, $j-$i));
		}
		$this->_next->visitFragment($fragment, $field, $weight);
	}
}
