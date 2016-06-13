
$(document).ready(function() {

	if ($.browser.mozilla) {
		$("div#statistics table").wrap("<div id='table_wrapper' style='padding: 0px; margin: 0px; width: 420px;'></div>");
		$("div#statistics div#table_wrapper").corner("round");
	}
});