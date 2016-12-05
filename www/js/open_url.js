//if page comes from URL
 if(window.location.hash != ''){
  var url = window.location.hash;
  url = '..'+url.substr(1, url.length);
  loadContent(url);
 }
 //if page comes from an internal link
 $("a:not([target])").click(function(e){
  e.preventDefault();
  var url = $(this).attr("href");
  if(url != '#'){
   loadContent($(this).attr("href"));
  }
 });


//LOAD CONTENT
function loadContent(url) {

  var contentContainer = $("#wrapper-window");

      if(contentContainer.is(":hidden")){
				$(contentContainer).show(); 
  		}
  		
	  //set load animation
  $(contentContainer).ajaxStart(function() {
  		$(this).html('Loading...');
  });

  $.ajax({
	   url: url,
	   dataType: "html",
	   success: function(data){

	   	   //store data globally so it can be used on complete
		    window.data = data;
	   },
	   complete: function(){
		    var content = $(data).find("#wrapper-window").html();
	  	 var contentTitle = $(data).find("title").text();

		 	 //change url
		    var parsedUrl = url.substr(2,url.length)
		    window.location.hash = parsedUrl;

			  //change title
		    var titleRegex = /<title>(.*)<\/title>/.exec(data);
		    contentTitle = titleRegex[1];
		    document.title = contentTitle;

			//renew content
		    $(contentContainer).fadeOut(function(){
	    		 $(this).html(content).fadeIn();
		    });
		}
    });
}
