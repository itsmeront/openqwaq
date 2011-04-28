// JavaScript Document

$(document).ready(function() {

// Put default text in username box on login screen
	$('#user').each(function() {
    var default_value = this.value;
    $(this).css('color', '#666'); // this could be in the style sheet instead
    $(this).focus(function() {
        if(this.value == default_value) {
            this.value = '';
            $(this).css('color', '#333');
        }
    });
    $(this).blur(function() {
        if(this.value == '') {
            $(this).css('color', '#666');
            this.value = default_value;
        }
    });
});
						   
	// Match heights of login instructions columns 
	 $().matchHeights({  
         'elements': '#column_one, #column_two, #column_three' 
     });  

	// Create alternating shading on recent activity table and unordered lists
	$('#content_sub ul ul li:odd').addClass('shade');
	$('#recent_activity td.date').each(function(){
			breakDateTime($(this));	/* Format the date/time */
	});
	
	// Add some elements to cells besides td.item so the dom structure matches
	$('#recent_activity td').not('td.item').each(function() {
		$(this).wrapInner('<span></span>').prepend('<span></span>');
	 });
	
	// If lists are too tall, add class "tall" to add max-height and scrollbars
	$('#orgs_list').each(function(){
		if ($(this).height() > 500) {
			$(this).addClass('tall');	
		}
	  });
	$('#forums_list').each(function(){
		if ($(this).height() > 500) {
			$(this).addClass('tall');	
		}
	  });
	 
	// Check number of columns so you know which one to set the default sort on
	var numberOfColumns = $('#recent_activity th').length;
	if (numberOfColumns > 3) {
	$('#recent_activity').tablesorter({
		sortList: [[3,1]],textExtraction: myTextExtraction, widgets:['zebra']	/* Sort table on 4th column, descending order */
	});
	} else {
	$('#recent_activity').tablesorter({
		sortList: [[2,1]], textExtraction: myTextExtraction, widgets:['zebra']	/* Sort table on 3rd column, descending order */						  
	});
	}
	
	// Create list navigation
/*	$('#forums_list').listnav();
	$('#orgs_list').listnav();
*/	
});

function breakDateTime(s) {
	// Inserts a space into UTC date/time so it wraps. Also removes +00:00 from the end.
	var str = s.text();
	str = str.replace('T', ' ');
	str = str.replace('+00:00', '');
	s.text(str);

}

var myTextExtraction = function(node)  
{  
		// extract data from markup and return it  
		return node.childNodes[1].innerHTML; 
} 