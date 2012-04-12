(function ($) {
	
Drupal.ajax.prototype.commands.gmapRepaint = function (ajax, response, status) {
	
	var mapid = jQuery( response.selector + ' .gmap').attr('id').replace(/gmap-(.*)-gmap./,"$1");
	
	var map = Drupal.gmap.getMap( mapid );
	if ( map ) {
		map.change('clearmarkers',-1);
		map.change('iconsready',-1);
	}
	
};

})(jQuery);
