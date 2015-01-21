<?php

/**
 * Genetic evaluator interface
 *
 * @access		public
 * @package		synd.core.index
 */
class ISyndGeneticEvaluator {
	/**
	 * @return	array
	 */
	function getDefaultGenes() {}
	
	/**
	 * Calculates the fitness value of a gene (higher is better)
	 * @param	array	Gene to evaluate
	 * @return	float
	 */
	function getFitness($gene) {}

	/**
	 * Perform crossover where random value(s) are swapped between genes
	 * @param	array	Gene
	 * @param	array	Gene
	 */
	function crossover(&$gene1, &$gene2) {}

	/**
	 * Mutates a gene where random value(s) are replaced or introduced in gene
	 * @param	array	Gene to mutate
	 * @param	float	Chance to mutate, 0 <= factor <= 1
	 */
	function mutate($gene, $factor = 0.25) {}
}

/**
 * Genetic algorithm optimizer
 *
 * Uses reproduction, tournament crossover and mutation to spawn new 
 * generations. Uses an ISyndGeneticEvaluator implementation to 
 * evaluate the fitness of genes.
 *
 * @access		public
 * @package		synd.core.index
 */
class SyndGeneticOptimizer {
	var $_evaluator	= null;
	var $_reproduce = 0.25;
	var $_results = array();

	var $_size = 30;
	var $_population = array();
	var $_sizeTournament = 7;

	/**
	 * @param	ISyndGeneticEvaluator
	 * @param	integer	Size of population
	 * @param	float	Percent of population to reproduce into next generation (0 < $size < 1)
	 */
	function SyndGeneticOptimizer($evaluator, $size = 30, $reproduce = 0.25) {
		$this->_evaluator = $evaluator;

		$this->_size = $size;
		$this->_reproduce = $reproduce;

		// Initialize population
		foreach ($default = $this->_evaluator->getDefaultGenes() as $gene)
			$this->_population[SyndLib::hash($gene)] = $gene;
		
		for ($i=0; count($this->_population) < $this->_size && $i < 500; $i++) {
			$gene = $this->_evaluator->mutate($default[array_rand($default)], 0.75);
			$this->_population[SyndLib::hash($gene)] = $gene;
		}
	}

	/**
	 * Runs the model for a number of generations.
	 * @param	integer	Number of generations to run
	 */
	function run($maxGenerations = 100) {
		for ($generation=1; $generation <= $maxGenerations; $generation++) {
			print "Evaluating generation $generation\n";
			
			// Evalutate population
			foreach ($this->_population as $key => $gene) {
				if (!isset($this->_results[$key]))
					$this->_results[$key] = $this->_evaluator->getFitness($gene);
			}

			// Reproduce population
			$nextGeneration = array();
			arsort($this->_results);
			reset($this->_results);
			
			for ($i=0; count($nextGeneration) < $this->_size * $this->_reproduce && $i < count($this->_results); $i++) {
				list($key, $result) = each($this->_results);
				$nextGeneration[$key] = $this->_population[$key];
			}
			
			print "\n***** Finished generation $generation ***** \n";
			print "Result\tGene\n";
			foreach (array_keys($nextGeneration) as $key) 
				print round($this->_results[$key],4)."\t".SyndGeneticOptimizer::implode(" ",$nextGeneration[$key])."\n";
			print "\n\n";

			// Perform tournament crossover
			for ($i=0; count($nextGeneration) < $this->_size && $i < 500; $i++) {
				list ($gene1, $gene2) = $this->tournament($this->_population, $this->_results);
				$nextGeneration[SyndLib::hash($gene1)] = $gene1;
				$nextGeneration[SyndLib::hash($gene2)] = $gene2;
			}
			
			// Store new population and reset results
			$this->_population = $nextGeneration;
			$this->_results = SyndLib::array_kintersect(
				$this->_results, $this->_population);
		}
	}
	
	function implode($glue, $gene) {
		$str = null;
		foreach ($gene as $key => $value) {
			if (is_array($value)) {
				if (null != $str)
					$str .= ', ';
				$str .= SyndGeneticOptimizer::implode($glue, $value);
			}
			else {
				if (null != $str)
					$str .= $glue;
				$str .= $value;
			}
		}
		return "($str)";
	}
	
	/**
	 * Performs a tournament crossover.
	 *
	 * Six genes are choose by random and the best two are submitted to 
	 * crossover and mutation and subsequently returned. Population and
	 * results must share the same array keys.
	 *
	 * @param	array	Entire current population
	 * @param	array	Results of current population
	 * @return	array	array(gene1, gene2)
	 */
	function tournament($population, $results) {
		$tournament = array();
		for ($i=0; count($tournament) < $this->_sizeTournament && $i < 500; $i++) {
			$key = array_rand($population);
			$tournament[$key] = $population[$key];
		}
		
		// Inbreeding if tournament is not >= 2
		if (count($tournament) < 2)
			return array(reset($population), reset($population));

		$results = SyndLib::array_kintersect($results, $tournament);
		arsort($results);
		
		$keys  = array_keys($results);
		$gene1 = $tournament[array_shift($keys)];
		$gene2 = $tournament[array_shift($keys)];
		
		$this->_evaluator->crossover($gene1, $gene2);
		$gene1 = $this->_evaluator->mutate($gene1);
		$gene2 = $this->_evaluator->mutate($gene2);
		
		return array($gene1, $gene2);
	}
}

/**
 * Mediates between the genetic evaluator, the extension to test 
 * (weigthing scheme) and the evaluator implementation (trec9).
 */
class SyndGeneticMediator extends ISyndGeneticEvaluator {
	var $_evaluator = null;
	var $_index = null;
	var $_extension = null;

	/**
	 * @param	Trec9FitnessEvalutor
	 * @param	AbstractIndex			The index to run queries against
	 * @param	SyndIndexExtension	The extenstion to evaluate
	 */
	function SyndGeneticMediator($evaluator, $index, $extension) {
		$this->_evaluator = $evaluator;
		$this->_index = $index;
		$this->_extension = $extension;
	}
	
	/**
	 * Evaluates the fitness of a gene (higher values are better)
	 * @return	float
	 */
	function getFitness($gene) {
		$this->_extension->loadGene($gene);
		print "Gene: ".implode(', ', $gene)."\n";
		return $this->_evaluator->getFitness($this->_index);
	}
	
	function getDefaultGenes() {
		return $this->_extension->getDefaultGenes();
	}

	function getMinimumGene() {
		return $this->_extension->getMinimumGene();
	}

	function getMaximumGene() {
		return $this->_extension->getMaximumGene();
	}

	function crossover(&$gene1, &$gene2) {
		foreach (array_keys($gene1) as $key) {
			if (1 == mt_rand(1,count($gene1))) {
				$tmp = $gene1[$key];
				$key2 = array_rand($gene2);
				$gene1[$key] = $gene2[$key2];
				$gene2[$key2] = $tmp;
			}
		}
	}
	
	function mutate($gene, $factor = 0.25) {
		$min = $this->getMinimumGene();
		$max = $this->getMaximumGene();
		
		$factor = round(count($min)/$factor);
		foreach (array_keys($gene) as $key) {
			// Mutate this single value
			if (1 == mt_rand(1,$factor)) {
				// Fully random mutation
				if (1 == mt_rand(1,5)) 
					$gene[$key] = mt_rand(round($min[$key]*1000), round($max[$key]*1000))/1000;
				// Offset mutation, favors small modifications
				else {
					do {
						$offset = pow(mt_rand(round($min[$key]*1000), round($max[$key]*1000))/2000, 4);
						$offset = $offset < 1 ? $offset : 1/$offset;
						$gene[$key] += (1 == mt_rand(1,2) ? $offset : -$offset);
					} while ($gene[$key] < $min[$key] || $gene[$key] > $max[$key]);
				}
			}
		}
		return $gene;
	}
}

/**
 * Trec9 index evaluator.
 *
 * Reads queries and query relevance scores from files. Takes an
 * index and runs the queries to produce a fitness value indicating
 * how good the specific index setup performed.
 */
class Trec9IndexEvaluator {
	var $_queries = array();
	var $_answers = array();
	
	function Trec9IndexEvaluator($queryFile, $resultFile) {
		if (!file_exists($queryFile))
			return trigger_error("Query file '$queryFile' not found", E_USER_ERROR);
		if (!file_exists($resultFile))
			return trigger_error("Result file '$resultFile' not found", E_USER_ERROR);
			
		// Match for queries
		$text = file_get_contents($queryFile);
		preg_match_all('/<num>\s*Number:\s*([^\s]+)/', $text, $numbers);
		preg_match_all('/<title>\s*(.+)/', $text, $titles);
		preg_match_all('/<desc>\s*Description:\s*\n([^\n]+)/', $text, $descriptions);
		
		// Include both query title and description in probalistic query?
		// Using just the description gave better results (...)
		foreach ($numbers[1] as $i => $number) 
			$this->_queries[$number] = $titles[1][$i].' '.$descriptions[1][$i];
//		foreach ($numbers[1] as $i => $number) 
//			$this->_queries[$number] = $descriptions[1][$i];
		
		// Match for answers
		$text = file_get_contents($resultFile);
		preg_match_all('/(\S+)\s+([^\s]+)/', $text, $answers);
		foreach ($answers[1] as $i => $number) {
			$this->_answers[$number][] = $answers[2][$i];
			$this->_answers[$number] = array_unique($this->_answers[$number]);
		}
		
		if (empty($this->_queries))
			return trigger_error("No queries extracted from '$queryFile'.", E_USER_ERROR);
		if (empty($this->_answers))
			return trigger_error("No query results extracted from '$resultFile'.", E_USER_ERROR);
	}

	function ffp4Fitness($mset, $rset) {
		$k8 = 7;
		$k9 = 0.982;
		$fitness = 0;
		
		foreach (array_values($mset) as $i => $docid) {
			if (in_array($docid, $rset))
				$fitness += $k8 * pow($k9, $i+1);
		}
		
		return $fitness;
	}

	function fitness($mset, $rset) {
		$fitness = 0;
		foreach (array_values($mset) as $i => $docId) {
			if (in_array($docId, $rset))
				$fitness += 1 + pow((count($mset)-$i) / count($mset), 3);
		}
		return $fitness / count($rset);
	}

	/**
	 * Runs all the loaded queries 
	 *
	 * Compares the top $limit results with the pre approved results
	 * from the $resultFile file. Returns the precision, recall and
	 * fitness for each query.
	 *
	 * @return	array
	 */
	function getStatistics($index, $limit = 250) {
		require_once 'core/lib/SyndDate.class.inc';
		
		$precision = array();
		$recall  = array();
		$fitness = array();
		$ts = time();
		
		print "Query\t\tPrecision\tRecall\tFitness\tRelevant (r)\tTotal (R)\tRetrived (n)\tTime\n";
		foreach ($this->_queries as $number => $query) {
			if (!isset($this->_answers[$number]))
				continue;
			
			print $number."\t";
			if (strlen($number) <= 6)
				print "\t";

			$qtime = microtime(true);
			if (SyndLib::isError($mset = $index->getMatchSet(new IndexQuery($query), 0, $limit))) {
				print_r($mset);
				continue;
			}
			
			$time = microtime(true)-$qtime;
			$mset = SyndLib::array_collect($mset, 'ID');

			$relevant = 0;
			foreach ($mset as $rank => $docId) {
				if (in_array($docId, $this->_answers[$number])) 
					$relevant++;
			}
			
			if (count($mset)) {
				$precision[] = $relevant / count($mset);
				$recall[]    = $relevant / count($this->_answers[$number]);
				$fitness[]   = $this->ffp4Fitness($mset, $this->_answers[$number]);
			} 
			else {
				$precision[] = 0;
				$recall[]    = 0;
				$fitness[]   = 0;
			}

			print round(end($precision),2)."\t\t";
			print round(end($recall),2)."\t";
			print round(end($fitness),2)."\t";
			print $relevant."\t\t";
			print count($this->_answers[$number])."\t\t";
			print count($mset)."\t\t";
			print round($time,3)."s\n";
		}

		$time = time()-$ts;
		print "Medium precision: ".round(array_sum($precision)/count($precision),4);
		print ", recall: ".round(array_sum($recall)/count($recall),4);
		print ", fitness: ".round(array_sum($fitness)/count($fitness),4);
		print ", time: ".floor($time/60)."m ".($time%60)."\n\n";
		
		return array($precision, $recall, $fitness);
	}
	
	/**
	 * Evaluates the fitness of an index implementation (higher fitness values is better)
	 * @param	AbstractIndex
	 * @param	integer	The top $limit results to retrieve
	 * @return	float
	 */
	function getFitness($index, $limit = 250) {
		list ($precision, $recall, $fitness) = $this->getStatistics($index, $limit);
		return array_sum($fitness)/count($fitness);
	}
}

/**
 * Evaluates the fitness of a hash function
 *
 * The hash function is defined as a series of postfix operations 
 * that is performed on a value. The number of collisions and the
 * cost (execution time, ...) of the hash function determine its 
 * fitness.
 */
class HashFunctionEvaluator extends ISyndGeneticEvaluator {
	var $_terms = null;
	var $_stack = array();
	var $_length = 16;
	
	/**
	 * @param	array	The terms to test the hash functions on
	 */
	function HashFunctionEvaluator($terms) {
		$this->_terms = array_values(array_filter($terms));
		
		// Begin small
		$this->_terms = array_slice($this->_terms, 0, 7500);
	}
	
	/**
	 * Evaluates the fitness of a hash function (higher fitness values is better)
	 * @param	array
	 */
	function getFitness($gene) {
		static $functions = array();
		$key = SyndLib::hash($gene);
		if (!isset($functions[$key])) {
			if (count($functions) > 50)
				array_splice($functions, 20);
			$functions[$key]['loop'] = $this->_compile($gene['loop']);
			$functions[$key]['post'] = $this->_compile($gene['post']);
		}
		
		$loop = $functions[$key]['loop'];
		$post = $functions[$key]['post'];

		$hashes = array();
		$distinct = array();
		$collisions = 0;
		
		$prev = 0;
		$prevSecond= 0;
		
		$context = null;
		$context2 = null;
		
		$ts1 = microtime();

		for ($i=0; $i<count($this->_terms); $i++) {

			// Check if context collides for this term
			$hash = $prev | ($prevSecond >> 1);
			if (!isset($hashes[$this->_terms[$i]])) {
				$hashes[$this->_terms[$i]] = array(
					'hash' => $hash,
					'context' => array($context));
			}
			else if ($i >= 1) {
				if (!in_array($this->_terms[$i-1], $hashes[$this->_terms[$i]]['context'])) {
					if (($hashes[$this->_terms[$i]]['hash'] & $prev) == $prev)
						$collisions += 2;
					$hashes[$this->_terms[$i]]['context'][] = $this->_terms[$i-1];
					$hashes[$this->_terms[$i]]['hash'] |= $prev;
				}

				if ($i >= 2) {
					$context = $this->_terms[$i-1].'.'.$this->_terms[$i-2];
					$hash = $prev | ($prevSecond >> 1);

					if (!in_array($context, $hashes[$this->_terms[$i]]['context'])) {
						if (($hashes[$this->_terms[$i]]['hash'] & $hash) == $hash)
							$collisions += 2;
						$hashes[$this->_terms[$i]]['context'][] = $context;
						$hashes[$this->_terms[$i]]['hash'] |= $hash;
					}
				}
			}
			
			// Calculate hash for current term
			$prevSecond = $prev;

			$stack = array(0);
			for ($j=0; $j<strlen($this->_terms[$i]); $j++)
				$loop($stack, $this->_terms[$i], $j);
			$post($stack, $this->_terms[$i], 0);
			$hash = end($stack);
			
//			print_r($stack);
//			print $prev;
			
			if (null === $hash || false === $hash || '' === $hash || !is_numeric($hash)) {
				print "Function returned invalid ($hash) for '{$this->_terms[$i]}': ".tpl_chop(SyndGeneticOptimizer::implode(" ", $gene),76)."\n";
				return 0;
			}

			$prev = ((int)$hash) & 0xFFFFFF;

			if (isset($distinct[$prev]) && $distinct[$prev] != $this->_terms[$i])
				$collisions++;
			else
				$distinct[$prev] = $this->_terms[$i];

		}

		$ts2 = microtime();
		list($m1, $s1) = explode(' ', $ts1);		
		list($m2, $s2) = explode(' ', $ts2);
		
		$cost = ($s2 + $m2 - $s1 - $m1);
		$fitness = $this->fitness($collisions, $cost);
		
		print "$collisions coll, fit ".round($fitness,4)." ".round($cost,2)." secs ".
			tpl_chop(SyndGeneticOptimizer::implode(" ", $gene),100)."\n";
		
		return $this->fitness($collisions, $cost);
	}

	function fitness($collisions, $cost) {
		$k8 = 7;
		$k9 = 0.982;
		return $k8 * pow($k9, $collisions + 1) / ($cost * 0.1 + 1);
	}

	/**
	 * Some default hash functions
	 * @link	http://burtleburtle.net/bob/hash/doobs.html
	 */
	function getDefaultGenes() {
		$genes = array();

  		$genes[] = array(
			'loop' => array(),
			'post' => array('1', '20448', 'term', '.', 'crc32', 24, '%', '<<'));

  		$genes[] = array(
			'loop' => array(),
			'post' => array(
				'-', 'top', 'term', '.', '10700', 'ord', '15', '&', 'term', '15', '1', '<<', 'term', 
				'.', 'md5', 'crc32', '15', 'end', '%', '|', '1', '2', 'term', 'md5', 'crc32', '18695', 
				'ord', '+', '%', '1', '3', 'term', '.', 'md5', 'crc32', '15', '&', '<<'
				));

  		$genes[] = array(
			'loop' => array(),
			'post' => array(
				'^', '%', '-', 'i', '<<', '1', 'strlen', '<<', '|', 'i', 'top', '>>', '-', '47429', '<<', 'crc32', 
				'^','md5', 'i', 'strlen', 'i', '+', 'top', '-', '&', 'term', 'crc32', '63238', 'crc32', '<<', 'top', 
				'top', '<<', '+', '+'
				));

  		$genes[] = array(
			'loop' => array(),
			'post' => array(
				'-', '|', 'hash', '+', '%', '%', '|', '&', '>>', '|', '&', '47429', '|', 'term', '>>', '46499', '|', 
				'hash', 'i', '&', 'term', 'crc32', 'crc32', '63238', 'crc32', '<<', 'top', 'top', '<<', '^'
				));

  		$genes[] = array(
			'loop' => array(),
			'post' => array(
				'top', '%', '-', 'i', '<<', '1', 'strlen', '<<', '|', 'i', 'top', '>>', '-', '47429', '<<', 'crc32', 
				'^', 'md5', 'i', 'top', 'strlen', 'i', '+', 'top', '-', '&', '%', 'term', 'crc32', '63238', 'crc32', 
				'<<', 'top', 'top', '<<', '+', '+'
				));

		// Phrase hash function 
		$genes[] = array(
			'loop' => array(),
			'post' => array(
				1, 0,'term','.','md5','crc32', 24,'%', '<<', '|',
				1, 1,'term','.','md5','crc32', 24,'%', '<<', '|', 
				1, 2,'term','.','md5','crc32', 24,'%', '<<', '|', 
				1, 3,'term','.','md5','crc32', 24,'%', '<<', '|', 
				));

		$genes[] = array(
			'loop' => array(),
			'post' => array(
				1, 0,'term','.','crc32','crc32', 24,'%', '<<', '|',
				1, 1,'term','.','crc32','crc32', 24,'%', '<<', '|', 
				1, 2,'term','.','crc32','crc32', 24,'%', '<<', '|', 
				1, 3,'term','.','crc32','crc32', 24,'%', '<<', '|', 
				));

		// Rotating Hash
  		$genes[] = array(
			'loop' => array('hash',4,'<<', 'hash',28,'>>', '^', 'ord','^'),
			'post' => array(
				1, 'hash',1613,'%', 24,'%', '<<', '|',
				1, 'hash',4787,'%', 24,'%', '<<', '|',
				1, 'hash',6263,'%', 24,'%', '<<', '|',
				1, 'hash',7919,'%', 24,'%', '<<', '|'));
		
		// One-at-a-Time Hash
  		$genes[] = array(
			'loop' => array('ord','+', 'top',10,'<<','+', 'top',6,'>>','^'),
			'post' => array(
				1, 'hash', 'end',3,'<<','+', 'end',11,'>>','^', 'end',15,'<<','+', 1613,'%', 24,'%', '<<', '|',
				1, 'hash', 'end',3,'<<','+', 'end',11,'>>','^', 'end',15,'<<','+', 4787,'%', 24,'%', '<<', '|',
				1, 'hash', 'end',3,'<<','+', 'end',11,'>>','^', 'end',15,'<<','+', 6263,'%', 24,'%', '<<', '|',
				1, 'hash', 'end',3,'<<','+', 'end',11,'>>','^', 'end',15,'<<','+', 7919,'%', 24,'%', '<<', '|',
				));
		
		return $genes;
	}
	
	/**
	 * Compiles a gene (a set of postfix expressions) into a stackbased lambda function
	 * @return	string
	 */
	function _compile($gene) {
		$function = '';
		
		foreach ($gene as $op) {
			if (is_numeric($op))
				$function .= '$stack[] = '.$op.';';
			else {
				switch ($op) {
					case '.':
						$function .= '$rhs = array_pop($stack); $stack[] = array_pop($stack).$rhs;';
						break;
					case 'term':
						$function .= '$stack[] = $term;';
						break;
					case 'i':
						$function .= '$stack[] = $i;';
						break;
					case 'ord':
						$function .= '$stack[] = ord($term[$i]);';
						break;
					case 'top':
						$function .= '$stack[] = reset($stack);';
						break;
					case 'end':
						$function .= '$stack[] = end($stack);';
						break;
					case 'hash':
						$function .= '$stack[] = isset($hash) ? $hash : $hash = array_shift($stack);';
						break;
					case 'strlen':
						$function .= '$stack[] = strlen($term);';
						break;
					case 'md5':
						$function .= '$stack[] = md5(array_pop($stack));';
						break;
					case 'crc32':
						$function .= '$stack[] = crc32(array_pop($stack));';
						break;
					case 'strlen':
						$function .= '$stack[] = strlen($term);';
						break;
					case '<<':
						$function .= '$rhs = array_pop($stack); $stack[] = array_pop($stack) << $rhs;';
						break;
					case '>>':
						$function .= '$rhs = array_pop($stack); $stack[] = array_pop($stack) >> $rhs;';
						break;
					case '&':
						$function .= '$rhs = array_pop($stack); $stack[] = array_pop($stack) & $rhs;';
						break;
					case '|':
						$function .= '$rhs = array_pop($stack); $stack[] = array_pop($stack) | $rhs;';
						break;
					case '^':
						$function .= '$rhs = array_pop($stack); $stack[] = array_pop($stack) ^ $rhs;';
						break;
					case '+':
						$function .= '$rhs = array_pop($stack); $stack[] = array_pop($stack) + $rhs;';
						break;
					case '-':
						$function .= '$rhs = array_pop($stack); $stack[] = array_pop($stack) - $rhs;';
						break;
					case '%':
						$function .= '$rhs = array_pop($stack); $stack[] = array_pop($stack) % $rhs;';
						break;
					default:
						trigger_error("Non numeric operand '$op'", E_USER_ERROR);
				}
			}
			
			$function .= "\n";
		}
		
		if ('' == $function)
			return create_function('', 'return 0;');
		
		return create_function('&$stack, $term, $i', $function);
	}

	function _crossover(&$gene1, &$gene2) {
		foreach (array_keys($gene1) as $key) {
			if (1 == mt_rand(1,count($gene1))) {
				if (null == ($key2 = array_rand($gene2)))
					$gene2[0] = $gene1[$key];
				else {
					$tmp = $gene1[$key];
					$gene1[$key] = $gene2[$key2];
					$gene2[$key2] = $tmp;
				}
			}
		}
	}
	
	function crossover(&$gene1, &$gene2) {
		$this->_crossover($gene1['loop'], $gene2['loop']);
		$this->_crossover($gene1['post'], $gene2['post']);
	}

	function mutate($gene, $factor = 0.25) {
		$gene['loop'] = $this->_mutate($gene['loop'], $factor);
		$gene['post'] = $this->_mutate($gene['post'], $factor, true);
		return $gene;
	}
	
	function _mutate($gene, $factor = 0.25, $empty = false) {
		$ops = array(
			'.',
			'i',
			'ord',
			'top',
			'end',
			'term',
			'hash',
			'strlen',
			'md5',
			'crc32',
			'<<','>>','&','|','^',
			'+','-','%'
			);
		$result = array();
		
		foreach (array_keys($gene) as $key) {
			if (mt_rand(1, 100) > $factor*100)
				$result[] = $gene[$key];
			else {
				$lower = $empty || count($gene) > 1 || count($result) > 1 ? 1 : 3;
				switch (mt_rand($lower, 10)) {
					// Remove a operator
					case 1:
					case 2:
						break;
					// Insert a new op
					case 3:
						$result[] = $gene[$key];
						$result[] = $ops[array_rand($ops)];
						break;
					// Insert a new value
					case 4:	
						$result[] = $gene[$key];
						$result[] = mt_rand(0x0000,0xFFFF);
						break;
					// Replace with another operator
					default:
						$result[] = $ops[array_rand($ops)];
				}
			}
		}
		
		return $result;
	}

	function _intMutate($gene, $factor = 0.25) {
		$min = 1;
		$max = 16;
		
		foreach (array_keys($gene) as $key) {
			// Mutate this single value
			if (1 == mt_rand(1,round(1/$factor))) {
				// Fully random mutation
				if (1 == mt_rand(1,5)) 
					$gene[$key] = mt_rand($min, $max);
				// Offset mutation, favors small modifications
				else 
					$gene[$key] += 1 == mt_rand(1,2) ? 1 : -1;
				
			}
		}
		
		return $gene;
	}
}

class SyndFitnessEvaluator {
	var $_index = null;
	var $_evaluator = null;

	function SyndFitnessEvaluator($index, $evaluator) {
		$this->_index = $index;
		$this->_evaluator = $evaluator;
	}
	
	function run($min = 1, $max = 250) {
		$results = array();
		
		for ($r=$min; $r<=$max; $r+=ceil($r/10)) {
			print "Limit (r): $r\n";
			list ($precision, $recall, $fitness) = $this->_evaluator->getStatistics($this->_index, $r);
			$results[$r] = array(
				'precision' => round(array_sum($precision)/count($precision), 3),
				'recall'  => round(array_sum($recall)/count($recall), 3),
				'fitness' => round(array_sum($fitness)/count($fitness), 3),);
			if (1 == round(array_sum($recall)/count($recall), 3)) {
				print "Reached 100% recall at r=$r.\n";
				break;
			}
		}
		
		// Sort by recall
		uasort($results, create_function('$a,$b',
			'return $a["recall"]>$b["recall"] ? 1 : $a["recall"]<$b["recall"] ? -1 : 0;'));
		$levels = array();
		
		print "Limit (r)\tRecall\tPrecision\tFitness\n";
		foreach ($results as  $r => $result) {
			$recall = (string)round($result['recall'],1);
			if (!isset($levels[$recall]) || $levels[$recall]['precision'] < $result['precision'])
				$levels[$recall] = $result;
			print "$r\t\t{$result['recall']}\t{$result['precision']}\t\t{$result['fitness']}\n";
		}
		
		ksort($levels);
		
		print "\nRecall\tPrecision\tFitness\n";
		foreach ($levels as  $recall => $result)
			print "$recall\t{$result['precision']}\t\t{$result['fitness']}\n";

		$threePointAvg = round((
			$levels['0.2']['precision'] +
			$levels['0.5']['precision'] + 
			$levels['0.8']['precision'])/3, 3);
		print "\n3-point average (0.2, 0.5, 0.8): $threePointAvg\n";
		
		$avg = round(array_sum(SyndLib::array_collect($levels, 'precision')) / count($levels), 3);
		print count($levels)."-point average: $avg\n";
	}
}
