$(function(){
	// Accordion
	$("#panel").accordion({ header: "h2" });

	// Tabs
	$('#tabs').tabs();
	

	// Dialog			
	$('#dialog').dialog({
		autoOpen: false,
		width: 600,
		buttons: {
			"Ok": function() { 
				$(this).dialog("close"); 
			}, 
			"Cancel": function() { 
				$(this).dialog("close"); 
			} 
		}
	});
	
	// Dialog Link
	$('#dialog_link').click(function(){
		$('#dialog').dialog('open');
		return false;
	});

	// Datepicker
	$('#datepicker').datepicker({
		inline: true
	});
	
	// Slider
	$('#slider').slider({
		range: true,
		values: [17, 67]
	});
			
	// Progressbar
	$("#progressbar").progressbar({
		value: 20 
	});
				
	//hover states on the static widgets
	$('#dialog_link, ul#icons li').hover(
		function() { $(this).addClass('ui-state-hover'); }, 
		function() { $(this).removeClass('ui-state-hover'); }
	);
			
	$( "#draggable-panel" ).draggable();
	$( "#draggable-panel-tabbed" ).draggable();
	$( "#draggable-wrapper-window" ).draggable();
	$( "#draggable-clock" ).draggable();
		
	$( "#draggable-panel" ).resizable();
	$( "#draggable-panel-tabbed" ).resizable();
	$( "#draggable-wrapper-window" ).resizable();
	$( "#draggable-clock" ).resizable();

	$('.sortable').sortable();

// Open windows
   $('a.open').click(function(e){
      e.preventDefault()
      $($(this).attr('href')).show();
   });   
     
// Maximize windows
	 $('a.maximize').click(function(e){
      e.preventDefault()
      $($(this).attr('href')).animate({
	        position: "absolute",
	        top: 5,
	        left: 10,
	        height: '95%',
	        width: '95%',
	        opacity: 1,
	        easing: 'easeOutCubic'
      });
   });

// Minimize windows
   $('a.minimize').click(function(e){
      e.preventDefault()
      $($(this).attr('href')).slideToggle();
   });

// Close windows
   $('a.close').click(function(e){
      e.preventDefault()
      $($(this).attr('href')).hide();
   });   
     
});
