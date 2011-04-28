/**
  * jQuery listify plugin (version 1.0)
  *
  * http://www.dyve.net/jquery/?listify
  *
  * @name  listify
  * @type  jQuery
  * @param Hash    settings             settings 
  * @param String  settings[selector]   selector to use
  * @param String  settings[hoverClass] class to apply to items that the mouse is over an item
  * @param String  settings[cursorType] cursor type to change to when the mouse is over an item
  * @param Bool    settings[hideLink]   whether or not to hide the link
  */
(function($) {
	$.fn.listify = function(settings) {
		settings = $.extend({
			hoverClass: "over",
			cursorType: "pointer",
			selector: "tbody tr",
			hideLink: false
		}, settings);
		$(settings.selector, this).each(function() {
			var anchor = $("a", this);
			if (anchor.length == 1) {
				anchor = $(anchor.get(0));
				var thickbox = anchor.is(".thickbox");
				var link = anchor.attr("href");
				if (link) {
					if (settings.hideLink) {
						var text = anchor.html();
						anchor.after(text).hide();	
					}
					$(this)
						.hover(
							function() { $(this).addClass(settings.hoverClass) },
							function() { $(this).removeClass(settings.hoverClass) }
						)
						// apply link (and hand pointer) to all td which do not contain a select element
						.find("td:not('td:has(select)')")
						.css("cursor", settings.cursorType).click(function() {					  
								if (thickbox) {
									anchor.click();
								} else {
									window.location.href = link;
								}
				   });
				}
			}
		});
		return this;
	}
})(jQuery)