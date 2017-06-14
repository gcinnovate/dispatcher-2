$(function(){
	$('.completeness_btn').click(function(){
        id_val = $(this).attr('id');
        $.get(
            '/api/v1/reportsthisweek/' + id_val,
            {},
            function(data){
                $('#modal_res4').html(data);
            });
    });
	$('#sendsms').click(function(){
        $('#modal_res2').css({'color': 'green'});
        facilityid = $('#facilityid').val()
        sms = $('#sms').val()
        role = $('#role').val()
        $.post(
            '/api/v1/facilitysms/' + facilityid,
            {sms:sms, role:role},
            function(data){
            $('#modal_res2').html(data);
        });
        return false;
    });
});
