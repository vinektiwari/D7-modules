
jQuery.noConflict();
jQuery(document).ready(function() {
	jQuery("#memdir-list").DataTable( {
		"pagingType": "simple_numbers",
		"lengthMenu" : [10, 50, 100, 500],
		"pageLength" : 10,
	})

	jQuery('.expand').click(function(e) {
		alert(jQuery('.expand').data("value"));
	})
});