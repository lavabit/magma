
$(document).ready(function() {

	if ($.browser.mozilla) {
		$("div#corporate_e_mail table#users").wrap("<div id='users_wrapper' style='padding: 0px; margin: 0px 0px 0px 0px; width: 300px;'></div>");
		$("div#corporate_e_mail div#users_wrapper").corner("round");
		
		$("div#corporate_e_mail table#plan").wrap("<div id='plan_wrapper' style='padding: 0px; margin: 0px; width: 400px;'></div>");
		$("div#corporate_e_mail div#plan_wrapper").corner("round");
	}
});