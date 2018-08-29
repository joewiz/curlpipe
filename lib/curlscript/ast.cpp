/******************************************************************************
 * curlscript - https://github.com/xquery/curlscript
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

using namespace std;

namespace curlscript {

    struct item generate_item(pugi::xml_node &item){
        struct item cur_item;
        cur_item.uri.scheme = item.child("URI").child("scheme").text().as_string();
        cur_item.uri.host = item.child("URI").child("hostport").child("host").child(
                "nstring").text().as_string();
        for(pugi::xml_node port: item.child("URI").child("hostport").child("port").children()) {
            cur_item.uri.port +=port.text().as_string();
        }

        for(pugi::xml_node path: item.child("URI").children("segment")) {
            cur_item.uri.path += "/";
            cur_item.uri.path += path.child("string").text().as_string();
        }
        DLOG_S(INFO) << "      uri:" << cur_item.uri.get_uri();
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
