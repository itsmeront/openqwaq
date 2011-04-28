// JavaScript Document

jQuery(function($) {
	$('.alternate').not('.nolistify').listify();
	$('.alternate tr:even, .alternate li:even').addClass('even');
	$('.alternate tr:odd, .alternate li:odd').addClass('odd');
	$('#account_info li:last-child a').css('border-right', 'none');
	
	$('fieldset.conf legend').append('<a href="#" class="show">Show</a>');
		   
	$('fieldset.conf textarea').hide();
	$('fieldset.conf').closest('form').find('input[type=submit]').hide();
	
	$('fieldset.conf legend a').click(function(e) {
		e.preventDefault();
		$(this).closest('fieldset').find('textarea').slideToggle();
		$(this).closest('form').find('input[type=submit]').toggle();
		if($(this).attr('class') == 'show') {
			$(this).removeClass('show').text('hide').addClass('hide');
		} else if($(this).attr('class') == 'hide') {
			$(this).removeClass('hide').text('show').addClass('show');
		}
	});
	$('textarea.conf:not(.processed)').TextAreaResizer();

	$('.actions input[type=image], .actions a').tooltip({
		fade: 250,
		delay: 400,
		showURL: false,
		track: true
	});
});
