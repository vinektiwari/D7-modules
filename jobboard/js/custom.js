
// jQuery(document).ready(function() {
// 	jQuery("#btnSearch").click(function(){
// 		alert("aaa");
// 	})
// });

(function($) {
    Drupal.behaviors.jobboard = {
        'attach': function(context) {
            $('#btnSearch', context).addClass('jobboard-processed').bind('click', function() {
				var key = jQuery("#keySearch").val();
				var type = jQuery("#typeSearch").val();
				
				$.ajax({
					type: "POST",
					url: "/jobboard-alljobs-block-content",
					data: "number1=" + number1 + "&number2=" + number2,
					success: function(response){
						alert(response);
					}
				});
				return false;
				// .post('/jobboard_alljobs_block_content', null, key);
				// return false;
            });
        }
    }
    var formData = function(response) {
        $('jobboard-search-result-id-wrapper').html(response.data);
    }
})(jQuery);