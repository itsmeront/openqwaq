(function ($) {
    // Create a namespace if it does not exist
	$.nrj = $.nrj ? $.nrj : {};
	$.nrj.matchHeights = function (options) {
		// Extend the default options with those provided.
		var opts = $.extend({}, $.nrj.matchHeights.defaults, options);
		// Return for chaining
		return this.each(function () {
			// Extend with meta plugin if it is included
			opts = $.meta ? $.extend({}, opts, $(this).data()) : opts;

			// Put elements into an array
			var els = opts.elements.split(',');
			if (els.length < 1) return false;

			var tallest = 0;
			var topHeight = 0;
			var newEls = [];

			// iterate over each element, adding its total height to an array,
			// setting the tallest element in the list
			$.each(els, function (i, val) {
				tempTop = $(val).offset().top;
				tempHeight = tempTop + $(val).height();
				newEls[i] = tempHeight;
				if (tempHeight > topHeight) {
					tallest = i;
					topHeight = tempHeight;
				}
			});

			// iterate over each element, matching the height of shorter els
			$.each(els, function (i, val) {
				if (i == tallest) return;
				temp = $(els[i]).height();
				$(els[i]).height(topHeight - newEls[i] + $(els[i]).height());
			});
		});
	}

	$.nrj.matchHeights.defaults = {
		elements: ""
	}

	$.fn.matchHeights = $.nrj.matchHeights;

})(jQuery);
