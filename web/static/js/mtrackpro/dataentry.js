$(function(){
    $('#district').change(function(){
        var districtid = $(this).val();
        if (districtid == '0' || districtid == "")
            return;
        $('#facility').empty();
        $('#facility').append("<option value='' selected='selected'>Select Health Facility</option>");
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

    $('#facility').change(function(){
        var facilityid = $(this).val();
        $('#reporter').empty();
        $('#reporter').append("<option value='' selected='selected'>Select Reporter</option>");
        $.get(
            '/api/v1/facility_reporters/' + facilityid,
            {},
            function(data){
                var reporters = data;
                for (var i in reporters){
                    val = reporters[i]["telephone"];
                    txt = reporters[i]["firstname"] + " " + reporters[i]["lastname"] + " (" + val + ")";
                    $('#reporter').append(
                        $(document.createElement("option")).attr("value",val).text(txt)
                    );
                }
            },
            'json'
        );
    });

    $('#report_type').change(function(){
        var report_type = $(this).val();
        $('#report').empty();
        $('#report').append("<option value='' selected='selected'>Select Report</option>");
        $.get(
            '/api/v1/reportforms/' + report_type,
            {},
            function(data){
                var reportforms = data;
                for (var i in reportforms){
                    val = reportforms[i]["name"];
                    /*txt = val.replace(/\b\w/g, l => l.toUpperCase()) + " Report";*/
                    txt = val.toUpperCase() + " Report";
                    $('#report').append(
                        $(document.createElement("option")).attr("value",val).text(txt)
                    );

                }

            },
            'json'
        );
    });

    $('#report').change(function(){
        var report = $(this).val();
        $('#dataentry_res').html("");
        $.get(
            '/api/v1/indicatorhtml/' + report,
            {},
            function(data){
                $('#dataentry_res').html(data);
            }
        );
    });
});
