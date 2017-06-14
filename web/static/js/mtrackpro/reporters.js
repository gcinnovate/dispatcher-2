$(function(){
    $('#district').change(function(){
        var districtid = $(this).val();
        if (districtid == '0' || districtid == "")
            return;
        $('#location').val(districtid);
        $('#subcounty').empty();
        $('#subcounty').append("<option value='' selected='selected'>Select Sub County</option>");
        $('#facility').empty();
        $('#facility').append("<option value='' selected='selected'>Select Health Facility</option>");
		$.get(
            '/api/v1/loc_children/' + districtid,
            {xtype:'district', xid: districtid},
            function(data){
                var subcounties = data;
                for(var i in subcounties){
                    val = subcounties[i]["id"];
                    txt = subcounties[i]["name"];
                    $('#subcounty').append(
                        $(document.createElement("option")).attr("value",val).text(txt)
                    );
                }
            },
            'json'
        );
        $.get(
            '/api/v1/district_facilities/' + districtid,
            {xtype:'district', xid: districtid},
            function(data){
                var facilities = data;
                for(var i in facilities){
                    val = facilities[i]["id"];
                    txt = facilities[i]["name"];
                    $('#facility').append(
                        $(document.createElement("option")).attr("value",val).text(txt)
                    );
                }
            },
            'json'
        );
    });

	$('#subcounty').change(function(){
        var subcountyid = $(this).val();
        if (subcountyid == '0' || subcountyid == '')
            return;
        $('#location').val(subcountyid);
        $('#facility').empty();
        $('#facility').append("<option value='' selected='selected'>Select Health Facility</option>");
        $.get(
            '/api/v1/loc_facilities/' + subcountyid,
            {},
            function(data){
                var facilities = data;
                for(var i in facilities){
                    val = facilities[i]['id'];
                    txt = facilities[i]['name'];
                    $('#facility').append(
                            $(document.createElement("option")).attr("value",val).text(txt)
                    );
                }
            },
            'json'
        );
	});
});
