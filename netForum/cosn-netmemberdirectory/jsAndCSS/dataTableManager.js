/**
 *
 * @param data - json encoded data
 * @param fieldsToDisplay - the rows to display
 */
function displayTable(data, count, attrs) {

    attMap = JSON.parse(attrs);

    var t;
    var y = data;
    var target_col = y.groupBy;
    if (target_col < 0) {
        if (attMap['scrollable'] == 1) {
            t = jq1113('#fsDataTable' + count).DataTable({
                "scrollY":        attMap["scroll_height"],
                "scrollCollapse": true,
                "paging":         false
            });
        }else{
            t = jq1113('#fsDataTable' + count).DataTable({
                "lengthMenu": [[10, 25, 50, -1], [10, 25, 50, "All"]],
            });
        }
        if (typeof(y.order_by) != "undefined" && y.order_by !== null) {
            t.order([y.order_by, 'asc']);
        }
    } else {
        var order = [[target_col, 'asc']];
        if (typeof(y.order_by) != "undefined" && y.order_by !== null) {
            order.push([y.order_by, 'asc']);
        }
        if(attMap['scrollable'] == 0) {
            t = jq1113('#fsDataTable' + count).DataTable({
                "columnDefs": [
                    {"visible": false, "targets": target_col}
                ],
                "order": order,
                "displayLength": 50,
                "lengthMenu": [[10, 25, 50, -1], [10, 25, 50, "All"]],

                "drawCallback": function (settings) {
                    var api = this.api();
                    var rows = api.rows({page: 'current'}).nodes();
                    var last = null;

                    api.column(target_col, {page: 'current'}).data().each(function (group, i) {
                        if (last !== group) {
                            jq1113(rows).eq(i).before(
                                '<tr class="netmemberdirectory_group"><td colspan="5">' + group + '</td></tr>'
                            );

                            last = group;
                        }
                    });
                }
            });
        }else{
            t = jq1113('#fsDataTable' + count).DataTable({
                "columnDefs": [
                    {"visible": false, "targets": target_col}
                ],
                "order": order,

                //"displayLength": 50,
                "scrollY":         attMap["scroll_height"],
                "scrollCollapse": true,
                "paging":         false,

                "drawCallback": function (settings) {
                    var api = this.api();
                    var rows = api.rows({page: 'current'}).nodes();
                    var last = null;

                    api.column(target_col, {page: 'current'}).data().each(function (group, i) {
                        if (last !== group) {
                            jq1113(rows).eq(i).before(
                                '<tr class="netmemberdirectory_group"><td colspan="5">' + group + '</td></tr>'
                            );

                            last = group;
                        }
                    });
                }
            });
        }

        // Order by the grouping
        jq1113('#fsDataTable' + count + ' tbody').on('click', 'tr.netmemberdirectory_group', function () {
            var currentOrder = t.order()[0];
            if (currentOrder[0] === target_col && currentOrder[1] === 'asc') {
                t.order([target_col, 'desc']).draw();
            }
            else {
                t.order([target_col, 'asc']).draw();
            }
        });

    }

    to_disp = y.fieldNames.split(",");

    for (i = 0; i < y.length; i++) {
        var j = 0;
        var tbl_arr = [];
        for (; j < to_disp.length; j++) {
            tbl_arr.push(y[i][to_disp[j]]);
        }
        t.row.add(tbl_arr).draw();
    }

}


/**
 * Displays a datatable. Data is retrieved via an ajax call
 * @param attrs - the shortcode attributes as a JSON encoded string
 * @param count - the dataTable being referenced
 */
function displayTableAjax(attrs, countIn) {

    jq1113(document).ready((function (count) {
        var errorVal = false;
        var data = null;
        jq1113.ajax({
            url: Drupal.settings.basePath + 'netmemberdirectory/ajax',//attrs['ajaxurl'],
            type: 'POST',
            /*contentType: "application/json",*/
            dataType: 'json',
            data: 'netmemberdirectory_table_ajax=' + attrs,
            success: function (data_in) {
                displayTable(data_in, count, attrs);
            },
            error: function (jqXHR, textStatus, errorThrown) {
                errorVal = textStatus;
            }
        });

        if (errorVal !== false) {
            console.error("ajax called failed: " + errorVal);
            return;
        }
    })(countIn));

}

