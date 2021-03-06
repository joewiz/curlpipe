/******************************************************************************
 * curlpipe - https://github.com/xquery/curlpipe
 ******************************************************************************
 * Copyright (c) 2017-2018 James Fuller <jim.fuller@webcomposite.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
******************************************************************************/

#include <vector>
#include "log.h"
#include "expr.h"
#include <pugixml.hpp>
#include <curl/curl.h>

using namespace std;

namespace curlpipe {

    struct item generate_item(pugi::xml_node &item){
        struct item cur_item;
        if(item.child("URI")){
            DLOG_S(INFO) << " encode item uri";
            CURLUcode rc;
            string scheme = item.child("URI").child("scheme").text().as_string();

            if(scheme.find(":") && !scheme.empty()){
                scheme = scheme.substr(0,scheme.find(":"));
                rc = curl_url_set(cur_item.uri.urlp, CURLUPART_SCHEME, scheme.c_str(),0);
            }else{
                rc = curl_url_set(cur_item.uri.urlp, CURLUPART_SCHEME, "file",0);
            }

            rc = curl_url_set(cur_item.uri.urlp, CURLUPART_HOST, item.child("URI").child("hostport").child("host").child(
                    "nstring").text().as_string(),0);

            string port;
            for(pugi::xml_node ports: item.child("URI").child("hostport").child("port").children()) {
                port += ports.text().as_string();
            }
            rc = curl_url_set(cur_item.uri.urlp, CURLUPART_PORT, port.c_str(), 0);

            string path = "";
            for(pugi::xml_node segment: item.child("URI").children("segment")) {
                path += "/";
                path += segment.child("pstring").text().as_string();
            }

            if(!path.empty()){
                rc = curl_url_set(cur_item.uri.urlp, CURLUPART_PATH, path.c_str(), 0);
            }else{
                DLOG_S(INFO) << "      url path is empty";
            }

            string fragment = "";
            for(pugi::xml_node fragments: item.child("URI").children("fragment")) {
                fragment = fragments.child("astring").text().as_string();;
            }
            if(!fragment.empty()){
                rc = curl_url_set(cur_item.uri.urlp, CURLUPART_FRAGMENT, fragment.c_str(), 0);
            }else{
                DLOG_S(INFO) << "      url fragment is empty";
            }

            string query = "";
            for(pugi::xml_node querys: item.child("URI").children("query")) {
                query = querys.child("astring").text().as_string();

            }
            if(!query.empty()){
                rc = curl_url_set(cur_item.uri.urlp, CURLUPART_QUERY, query.c_str(), 0);
            }else {
                DLOG_S(INFO) << "      url query is empty";
            }
            DLOG_S(INFO) << "      uri:" << cur_item.uri.get_uri();

            for(pugi::xml_node headers: item.child("URI").child("headers").children("header")) {
                string name = headers.child("literal").child("astring").text().as_string();
                string value = headers.child("literal").next_sibling("literal").child("astring").text().as_string();
                DLOG_S(INFO) << "      header " << name << ":"<< value;
                cur_item.headers.push_back(make_tuple(name,value));
            }
        }

        if(item.child("literal")) {
            DLOG_S(INFO) << " encode item literal";
            cur_item.literal = item.child("literal").child("astring").text().as_string();
        }

        if(item.child("selector")) {
            DLOG_S(INFO) << " encode item selector";
            cur_item.selector = item.child("selector").child("nstring").text().as_string();
        }

        return cur_item;
    }

    vector<expr> generate_ast(string parsed){
        DLOG_S(INFO) << "generate AST";
        pugi::xml_document doc;
        doc.load_string(parsed.c_str());
        std::vector<expr> exprs;
        pugi::xpath_node_set expressions = doc.select_nodes("/CS/Expr");

        int count =0;
        for (pugi::xpath_node_set::const_iterator it = expressions.begin(); it != expressions.end(); ++it)
        {
            DLOG_S(INFO) << "  Expr: ";

            expr cur;
            pugi::xpath_node node = *it;
            pugi::xml_node expression = node.node();

            for (pugi::xml_node expr_part: expression.children())
            {
                string name = expr_part.name();
                DLOG_S(INFO) << "    name:" << name;

                std::vector<item> items1;
                std::string op;
                std::vector<item> items2;

                if(name.compare("items") == 0){
                    for (pugi::xml_node item: expr_part.children()) {
                        if(item.child("URI")){
                            items1.push_back(
                                    generate_item(item));
                        }else if(item.child("var")){
                        }else if(item.child("literal")){
                            items1.push_back(
                                    generate_item(item));
                        }
                    }
                } else if(name.compare("statement") == 0){
                    op = expr_part.first_child().text().as_string();
                    DLOG_S(INFO) << "      op:" << op;
                    for (pugi::xml_node item: expr_part.child("items")) {
                        if(item.child("URI")){
                            items2.push_back(
                                    generate_item(item));
                        }else if(item.child("var")){
                        }
                    }
                }
                cur.statements.emplace_back(make_tuple(items1, op, items2)); }
                cur.order=count;
                exprs.push_back(cur);
                count++;}
    return exprs;}
}
