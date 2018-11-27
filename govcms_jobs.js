(function ($, Drupal) {
  Drupal.behaviors.govcmsJobs = {
    attach: function attach(context, setting) {
			govcmsjobs_form_js(context);
    }
	};

	function govcmsjobs_form_js(context){
		$('#edit-job-categories').select2({
			width: '440px',
			minimumResultsForSearch: 10
		});
		$('#edit-employer').select2({
			width: '440px',
			minimumResultsForSearch: 10
		});
		$('#edit-clearance-level').select2({
			width: '440px',
			minimumResultsForSearch: 10
		});
	}

})(jQuery, Drupal);

// Drupal.APSJobsRR.checkLengthCKEditor = function()
// {
// 	if (typeof (CKEDITOR) !== "undefined") {
// 		var editor = CKEDITOR.instances['edit-summary-skills-value'];
// 		editor.on('key', function (event) {
// 			var content = editor.document.getBody().getText();
// 			var wordCount = checkWordLimit(content);
// 			if (wordCount.words > 300) {
// 				var limitContent = getWords(content);
// 				editor.document.getBody().setText(limitContent);
// 				alert('The summary of statement of skills and ability should be <= 300 words');
// 				return false;
// 			}
// 		});
// 		editor.on('afterPaste', function () {
// 			var content = editor.document.getBody().getText();
// 			var wordCount = checkWordLimit(content);
// 			if (wordCount.words > 300) {
// 				var limitContent = getWords(content);
// 				editor.document.getBody().setText(limitContent);
// 				alert('The summary of statement of skills and ability should be <= 300 words');
// 				return false;
// 			}
// 		});
// 	}
// }
