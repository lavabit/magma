

$(document).ready(function() {

	if ($.browser.mozilla) {
		$("div#advertising table#impressions").wrap("<div id='impressions_wrapper' style='padding: 0px; margin: 0px; width: 250px;'></div>");
		$("div#advertising div#impressions_wrapper").corner("round");
	};
	
	if ($("#plan").val() == 100) {
		$("#total").html('$100.00');
	}
	else if ($("#plan").val() == 200) {
		$("#total").html('$198.00');
	}
	else if ($("#plan").val() == 500) {
		$("#total").html('$475.00');
	}
	else if ($("#plan").val() == 1000) {
		$("#total").html('$900.00');
	}
	
	$("input").focus(function() {
		$(this).removeClass("red");
		$(this).removeClass("white");	
		$(this).addClass("gray");
	});
	
	$("input").blur(function() {
		$(this).removeClass("gray");	
		$(this).addClass("white");
	});
	
	$("select#plan").change(function() {
		if ($("#plan").val() == 100) {
			$("#total").html('$100.00');
		}
		else if ($("#plan").val() == 200) {
			$("#total").html('$198.00');
		}
		else if ($("#plan").val() == 500) {
			$("#total").html('$475.00');
		}
		else if ($("#plan").val() == 1000) {
			$("#total").html('$900.00');
		}
	});
	
});