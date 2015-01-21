<?php
require_once 'unit/index/SyndIndexTestCase.class.inc';
require_once 'core/index/SyndNullIndex.class.inc';
require_once 'core/index/SyndStemFilter.class.inc';

class _index_StemFilter extends SyndIndexTestCase {
	function testEnglish() {
//		$dle = new SyndDLE();
//		$dle->clearBuildEnv();
//		$dle->clearExtensionCache('stem_en');
//		$dle->clearExtensionCache('stem_sv');

		$text = "
			caresses ponies pony ties caress cats
			feed agreed plastered bled motoring sing
			conflated troubled sized
			hopping tanned falling hissing fizzed failing filing
			happy sky
			relational conditional rational valency hesitancy digitizer
			conformably radically differently vilely analogously
			vietnamization predication operator feudalism decisiveness 
			hopefulness callousness formality sensitivity sensibility
			triplicate formative formalize electricity electrical hopeful goodness
			revival allowance inference airliner gyroscopic adjustable defensible
			irritant replacement adjustment dependent adoption homologou
			communism activate angularity homologous effective bowdlerize
			probate rate cease
			controll roll";
			
		$expected = "
			caress poni poni tie caress cat
			feed agre plaster bled motor sing
			conflat troubl size
			hop tan fall hiss fizz fail file
			happi sky
			relat condit ration valenc hesit digit
			conform radic differ vile analog
			vietnam predic oper feudal decis 
			hope callous formal sensit sensibl
			triplic format formal electr electr hope good
			reviv allow infer airlin gyroscop adjust defens
			irrit replac adjust depend adopt homologou
			commun activ angular homolog effect bowdler
			probat rate ceas
			control roll";

		$extension = new SyndStemFilter('en');
		$extension->initialize(new SyndNullIndex());
		$term = new SyndTermQuery($text);

		$extension->_callback_query_visit_term($result, $builder, $term);
		$this->assertEquals($expected, $term->toString());
	}

	function testSwedish() {
		$text = "
			AD-hair-day
			Symptom: Du drabbas av en nästan pervers lust att se slarvig ut,
			fast på ett snyggt sätt. Du ner flera timmar varje dag
			på att rufsa till håret på rätt sätt, estetiska hål i
			och leta efter t-shirts med ironiska motiv i
			Sjukdomen drabbar uteslutande manniskor som
			jobbar med design.
			Bot: klipp dig och skaffa ett annat jobb";
			
		$expected = "
			ad-hair-day
			symptom: du drabb av en nästan perver lust att se slarv ut,
			fast på ett snygg sätt. du ner fler timm varj dag
			på att rufs till håret på rätt sätt, estetisk hål i
			och let eft t-shirt med ironisk motiv i
			sjukdom drabb uteslut mannisk som
			jobb med design.
			bot: klipp dig och skaff ett ann jobb";
					
		$extension = new SyndStemFilter('sv');
		$extension->initialize(new SyndNullIndex());
		$term = new SyndTermQuery($text);

		$extension->_callback_query_visit_term($result, $builder, $term);
		$this->assertEquals($expected, $term->toString());
	}

	function testIndex() {
		$expected = "poni";
					
		$extension = new SyndStemFilter('en');
		$extension->initialize(new SyndNullIndex());
		$term = 'pony';
		
		$extension->_callback_index_posting(null, null, $term);
		$this->assertEquals($expected, $term);
	}
}

