def getPaginationString(_page, totalitems, limit, adjacents, targetpage, pagestring):
    def ceil(x, y):
        m = x % y
        return x / y + 1 if m > 0 else x / y

    if not adjacents:
        adjacents = 1
    if not limit:
        limit = 5
    if not _page:
        _page = 1

    _page = int(_page)
    limit = int(limit)
    adjacents = int(adjacents)
    margin = ""
    padding = ""

    prev = _page - 1
    _next = _page + 1
    lastpage = ceil(int(totalitems), limit)

    lpm1 = lastpage - 1
    pagination = ""
    pagination += "<div class='dataTables_info' role='status'> " + "%s" % _page + " of " + "%s" % lastpage + " Pages  </div>\n"
    if lastpage > 1:
        pagination += "<div id='mydata_paginate' style='float: right' class='dataTables_paginate paging_simple_numbers'"
        if margin or padding:
            pagination += " style='"
            if margin:
                pagination += "margin: " + margin
            if padding:
                pagination += "padding: " + padding
            pagination += "'"
        pagination += "><ul class='pagination'>"

        pagination += "<li class='paginate_button'><a href='" + targetpage + pagestring + "1'>&laquo; First</a></li>"
        if _page > 1:
            pagination += "<li class='paginate_button previous'><a href='" + "%s%s%s" % (
                targetpage, pagestring, prev) + "'>&lsaquo; Prev</a></li>"
        else:
            pagination += "<li class='paginate_button disabled'><a href='#'>&lsaquo; Prev</a></li>"
        if (lastpage < (7 + (adjacents * 2))):
            counter = 1
            while (counter < lastpage):
                if counter == _page:
                    pagination += "<li class=\" paginate_button active\"><a href='#'>%s</a></li>" % counter
                else:
                    pagination += "<li clas=\"paginate_button\"><a href=\"" + "%s%s%s" % (
                        targetpage, pagestring, counter) + "\">%s</a></li>" % counter
                counter += 1
        elif (lastpage > (7 + (adjacents * 2))):  # enough pages to hide some
            # close to beginning; only hide later page
            if (_page < 1 + (adjacents * 3)):
                counter = 1
                while (counter < (4 + (adjacents * 2))):
                    if counter == _page:
                        pagination += "<li class=\"paginate_button active\"><a href=''>%s</a></li>" % counter
                    else:
                        pagination += "<li class=\"paginate_button\"><a href=\"" + "%s%s%s" % (
                            targetpage, pagestring, counter) + "\">%s</a></li>" % counter
                    counter += 1
            elif (lastpage - (adjacents * 2) > _page and _page > (adjacents * 2)):
                counter = _page - adjacents
                while (counter < _page + adjacents):
                    if counter == _page:
                        pagination += "<li class=\"paginate_button active\"><a href='#'>%s</a></li>" % counter
                    else:
                        pagination += "<li class=\"paginate_button\"><a href=\"" + "%s%s%s" % (
                            targetpage, pagestring, counter) + "\">%s</a></li>" % counter
                    counter += 1
            # close to end;  only hide early pages
            else:
                counter = lastpage - (1 + (adjacents * 3))
                while (counter <= lastpage):
                    if counter == _page:
                        pagination += "<li class=\"paginate_button active\"><a href='#'>%s</a>" % counter
                    else:
                        pagination += "<li class=\"paginate_button\"><a href=\"" + "%s%s%s" % (
                            targetpage, pagestring, counter) + "\">%s</a></li>" % counter
                    counter += 1

        # next button
        if (_page < counter - 1):
            pagination += "<li class=\"paginate_button next\"><a href=\"" + targetpage + pagestring + "%s" % _next + "\">Next &rsaquo;</a></li>"
        else:
            pagination += "<li class=\"paginate_button disabled\"><span>Next &rsaquo;</span></li>"
        pagination += "<li class=\"paginate_button\"><a href=\"" + "%s%s%s" % (
            targetpage, pagestring, lastpage) + "\">" + "Last" + " &raquo;</a></li></ul></div>"
        # pagination += "<div class='dataTables_info' role='status'> " + "%s" % _page + " of " + "%s" % lastpage + " Pages  </div>\n"
    return pagination


def lit(**keywords):
    return keywords


def countquery(dbcon, dic):
    if not dic:
        return None
    if 'fields' not in dic or 'relations' not in dic:
        return None
    sql = "SELECT COUNT(*) AS c FROM %s" % dic['relations']
    if (dic.get('criteria', None)):
        sql += " WHERE %(criteria)s" % dic
    res = dbcon.query(sql)
    if res:
        return res[0]['c']
    return 0


def doquery(dbcon, dic):
    if not dic:
        return None
    if 'fields' not in dic or 'relations' not in dic:
        return None
    # build the query
    sql = "SELECT %(fields)s FROM %(relations)s " % dic
    if (dic.get('criteria', None)):
        sql += " WHERE %(criteria)s " % dic
    if (dic.get('order', None)):
        sql += " ORDER BY %(order)s " % dic
    if (dic.get('offset', None)):
        sql += " OFFSET %(offset)s " % dic
    if (dic.get('limit', None)):
        sql += " LIMIT %(limit)s " % dic
    res = dbcon.query(sql)
    return res
