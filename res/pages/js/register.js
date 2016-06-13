new function() {
	$.fn.validator = {
		init: function(o) {
      	if(o.name == 'username') { this.username(o) };
      	if(o.name == 'passone') { this.passone(o) };
      	if(o.name == 'passtwo') { this.passtwo(o) };
      	if(o.name == 'human') { this.human(o) };
      	
      },
      passone: function (o) {
      	var empty = /^$/;
			var pass = /^[\x21-\x7E]*$/;
			if (o.value.match(empty)) {
				doError(o, 'You must specify a password.');
			}
			else if (!o.value.match(pass)) {
				doError(o, 'At this time our system only supports passwords which consist of valid ASCII characters, excluding spaces and control codes.');
			} else {
				doSuccess(o);
			};
		},
		passtwo: function (o) {
			if (o.value == $("#passone").val()) {
				doSuccess(o);
			} else {
				doError(o, 'Please type your password a second time to make sure it was entered correctly.');
			};
		},
		human: function (o) {
			var human = /^[0-9a-zA-z]{10}$/;
			if (o.value.match(human)) {
				doSuccess(o);
			} else {
				doError(o, 'In order for us to verify that you are a human being and not an automated program we need you type the characters you see in the image below. The answer will be exactly ten characters and only contain letters and numbers.');
			};
		},
		username: function (o) {
			var name = /^[a-zA-z][0-9a-zA-z\_]*$/;
			if (o.value.match(name)) {
				doSuccess(o);
			} else {
				doError(o, 'The username you selected is invalid. The username must start with a letter and only contain letters, numbers and underscores. Please correct the problem and try again.');
			};
		}
	};
	
	function doSuccess(o) {
		$('#' + o.id).removeClass("red");
		$('#' + o.id).addClass("white");
		$('#' + o.id + '_msg').remove();  
	}
	 
	function doError(o,m) {
	 	$('#' + o.id).removeClass("white");
		$('#' + o.id).addClass("red");
		$('#' + o.id + '_msg').remove();
		$('#' + o.id).after('<div id="' + o.id + '_msg">' + m + '</div>');  
   }
};

function prettyMoney(amount) {
    amount -= 0;
    amount = (Math.round(amount * 100)) / 100;
    return (amount == Math.floor(amount)) ? amount + '.00' : ( (amount * 10 == Math.floor(amount * 10)) ? amount + '0' : amount);
}

$(document).ready(function() {

	if ($.browser.mozilla && document.getElementById("reg_step2") != null) {
		$("div#reg_step2 table#plans").wrap("<div id='plans_wrapper' style='padding: 0px; margin: 0px; width: 737px;'></div>");
		$("div#reg_step2 div#plans_wrapper").corner("round");
	}
	
	$("input").focus(function() {
		$(this).removeClass("red");
		$(this).removeClass("white");	
		$(this).addClass("gray");
	});
	
	$("input").blur(function() {
		$(this).removeClass("gray");	
		$(this).addClass("white");
		$(this).validator.init(this);
	});
	
	$("form").submit(function() {
		var output = true;
		$(this).find("input").blur();
		$(this).find(".red").each(function() { output = false; });
		return output;
	});
	
	$("form#step2").submit(function() {
		if ($("input#basic").attr("checked") != true && $("input#personal").attr("checked") != true) {
			$('div#plans_msg').remove();
			$('div#buttons').before('<div id="plans_msg"><br />Please select a plan before continuing.</div>');
			return false;  
		}
		return true;
	});
	
	$("table#plans input").click(function() {
		$('div#plans_msg').remove();
	});
});
