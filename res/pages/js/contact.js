new function() {
	$.fn.validator = {
		init: function(o) {
      	if(o.name == 'your_e_mail') { this.your_e_mail(o) };
      	if(o.name == 'your_name') { this.your_name(o) };
      	if(o.name == 'your_message') { this.your_message(o) };
      },
		your_e_mail: function(o) {
			var email  = /^([a-zA-Z0-9_\.\-])+\@(([a-zA-Z0-9\-])+\.)+([a-zA-Z0-9]{2,10})$/;
			if (o.value.match(email)) {
				doSuccess(o);
			} else {
				doError(o,'A valid e-mail address is required.');
			};
		},
		your_name: function (o) {
			var name = /\w+/;
			if (o.value.match(name)) {
				doSuccess(o);
			} else {
				doError(o,'Please enter your name.');
			};
		},
		your_message: function (o) {
			var message = /\w+/;
			if (o.value.match(message)) {
				doSuccess(o);
			} else {
				doError(o,'Please enter a message.');
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
		$('#' + o.id).after('<span id="' + o.id + '_msg">' + m + '</span>');  
     }
};

$(document).ready(function() {

	$("input").focus(function() {
		$(this).removeClass("red");
		$(this).removeClass("white");	
		$(this).addClass("gray");
	});
	
	$("textarea").focus(function() {
		$(this).removeClass("red");
		$(this).removeClass("white");	
		$(this).addClass("gray");
	});
	
	$("input").blur(function() {
		$(this).removeClass("gray");	
		$(this).addClass("white");
		$(this).validator.init(this);
	});
	
	$("textarea").blur(function() {
		$(this).removeClass("gray");	
		$(this).addClass("white");
		$(this).validator.init(this);
	});
	
	$("form").submit(function() {
		var output = true;
		$(this).find("#your_name, #your_e_mail, #your_message").blur();
		$(this).find(".red").each(function() { output = false; });
		return output;
	});
});