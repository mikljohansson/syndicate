<?php
require_once 'PHPUnit2/Framework/TestCase.php';

class _model_Node_OpenEndedText extends PHPUnit2_Framework_TestCase {
	function testCompileAnswerExpression() {
		$question = SyndNodeLib::factory('open_ended_text');

		$question->data['INFO_CORRECT_ANSWER'] = "You are always interrupting me! <It is/It's> really very rude.";
		$this->assertTrue($question->isCorrect("You are always interrupting me! It is really very rude."));
		$this->assertTrue($question->isCorrect("You are always interrupting me! It's really very rude."));

		$question->data['INFO_CORRECT_ANSWER'] = "You are always interrupting me! <It is/ It's > really very rude.";
		$this->assertTrue($question->isCorrect("You are always interrupting me! It's really very rude."));

		$question->data['INFO_CORRECT_ANSWER'] = "You are always interrupting me! < It is / It's >really very rude.";
		$this->assertTrue($question->isCorrect("You are always interrupting me! It's really very rude."));

		$question->data['INFO_CORRECT_ANSWER'] = "You are <always/often/mostly> interrupting me! <It is/It's> really very rude.";
		$this->assertTrue($question->isCorrect("You are always interrupting me! It's really very rude."));
		$this->assertTrue($question->isCorrect("You are often interrupting me! It's really very rude."));
		$this->assertTrue($question->isCorrect("You are mostly interrupting me! It's really very rude."));
	}
}
