
$(document).ready(function() {

	if ($.browser.mozilla) {
		$("div#health table").wrap("<div id='table_wrapper' style='padding: 0px; margin: 0px; width: 605px;'></div>");
		$("div#health div#table_wrapper").corner("round");
	}
});