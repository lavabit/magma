
$(document).ready(function() {
	if ($.browser.mozilla) {
		$("div#personal_e_mail table").wrap("<div id='wrapper' style='padding: 0px; margin: 0px; width: 737px;'></div>");
		$("div#personal_e_mail div#wrapper").corner("round");
	}
});