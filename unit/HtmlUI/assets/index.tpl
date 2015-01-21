<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
	<head>
		<title>Tests in '<?= $sq ?>'</title>
		<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
		<style type="text/css">
			<? $this->display('assets/index.css') ?>
		</style>
		<script type="text/javascript">
		<!--
			<? $this->display('assets/index.js') ?>
		//-->
		</script>
	</head>
	<body>
		<table class="Container">
			<tr>
				<td class="Sidebar">
					<table class="Folders">
						<thead>
							<tr>
								<th>Folders</th>
							</tr>
						</thead>
						<tbody>
							<tr>
								<td>
									<? if (!empty($dirs)) { ?>
									<ul>
										<li><a href="<?= $uripath ?>">Home</a>
										<?
										foreach ($dirs as $dir) {
											$this->display('assets/folder.tpl', array(
												'root' => $dir, 'options' => $options,
												'iterator' => new ParentIterator(new RecursiveDirectoryIterator($dir)),
												'expand' => rtrim($dir,DIRECTORY_SEPARATOR).DIRECTORY_SEPARATOR.$sq));
										}
										?>
										</li>
									</ul>
									<? } ?>
								</td>
							</tr>
						</tbody>
					</table>
					<form action="<?= htmlspecialchars($uripath) ?>" method="get">
						<input type="hidden" name="sq" value="<?= $sq ?>" />
						<table class="Options">
							<thead>
								<tr>
									<th>Options</th>
								</tr>
							</thead>
							<tbody>
								<tr>
									<td>
										<input type="checkbox" name="recurse" id="recurse" value="1" <?= 
											!empty($options['recurse'])?'checked="checked" ':'' ?> onchange="this.form.submit();" />
										<label for="recurse">Recurse for tests</label>
									</td>
								</tr>
								<tr>
									<td>
										<input type="checkbox" name="showok" id="showok" value="1" <?= 
											!empty($options['showok'])?'checked="checked" ':'' ?> onchange="this.form.submit();" />
										<label for="showok">Show OK tests</label>
									</td>
								</tr>
							</tbody>
						</table>
					</form>
				</td>
				<td>
					<? if (!empty($suites)) { ?>
					<form action="<?= htmlspecialchars($uri) ?>" method="post">
						<table class="Suites">
							<thead>
								<tr>
									<th class="Status" title="Toggle all suites">
										<input type="checkbox" onchange="toggleCheckboxes(this,document.getElementById('Suites').getElementsByTagName('input'));" <?= $toggle?'checked="checked" ':'' ?>/>
									</th>
									<th class="Title">Test name</th>
									<th class="Time">Time</th>
									<th>Message</th>
									<th class="Line">Line</th>
								</tr>
							</thead>
							<tbody id="Suites">
								<? foreach ($suites as $suite) { ?>
								<tr class="Suite <?= $suite['wasrun'] ? ($suite['result']->wasSuccessful() ? 'Success' : 'Failure') : 'Skipped' ?>">
									<td class="Status"><input type="checkbox" name="suites[]" value="<?= $suite['suite']->toString() ?>" id="<?= $suite['suite']->toString() ?>" <?= $suite['wasrun']?'checked="checked" ':'' ?>/>
									<td title="<?= $suite['suite']->toString() ?>"><label for="<?= $suite['suite']->toString() ?>"><?= preg_replace('/^_(\w+_)+/','',$suite['suite']->toString()) ?></label></td>
									<td><?= $suite['duration'] ? sprintf('%0.4f', $suite['duration']) : '' ?></td>
									<td class="Results" colspan="2">
										Tests: <?= $suite['suite']->count() ?> | 
										Failures: <?= $suite['result']->failureCount() ?> | 
										Errors: <?= $suite['result']->errorCount() ?>
									</td>
								</tr>
									<? foreach ($suite['tests'] as $result) { 
										switch ($result->getCode()) {
											case PHPUnit2_HtmlUI_TestResult::FAILURE: 
												$class = 'Failure'; 
												break;
											case PHPUnit2_HtmlUI_TestResult::SUCCESS: 
												if (!$options['showok'])
													continue 2;
												$class = 'Success'; 
												break;
											default:
												if (!$options['showok'])
													continue 2;
												$class = 'Skipped'; 
												break;
										}
										
										?>
										<tr class="Test <?= $class ?>">
											<td class="Status">&nbsp;</td>
											<td><?= $result->getTest()->getName() ?></td>
											<td><?= sprintf('%0.4f', $result->getDuration()) ?></td>
											<? 
											if (null != $result->getException()) { 
												if (strlen($message = $result->getException()->getMessage()) > 135)
													$message = substr($message,0,132).' ..';
												if (strlen($message) < 1)
													$message = '(no message)';

												$i = 0;
												$trace = $result->getException()->getTrace();
												while ($i < count($trace) && $trace[$i]['file'] != $suite['reflector']->getFileName())
													$i++;
											?>
											<td>
												<a href="javascript:toggle('<?= $id = md5(uniqid('')) ?>')"><?= htmlspecialchars($message) ?></a>
												<div class="Trace" id="<?= $id ?>">
													<p><?= htmlspecialchars($result->getException()->getMessage()) ?></p>
													<p><?= htmlspecialchars($result->getException()->getTraceAsString()) ?></p>
												</div>
											</td>
											<td><?= isset($trace[$i]['line']) ? $trace[$i]['line'] : '&nbsp;' ?></td>
											<? } else { ?>
											<td>&nbsp;</td>
											<td>&nbsp;</td>
											<? } ?>
										</tr>
									<? } ?>
								<? } ?>
							</tbody>
						</table>
						<input type="submit" value="Run tests" />
					</form>
					<? } ?>
				</td>
			</tr>
		</table>
	</body>
</html>
