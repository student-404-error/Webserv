/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RedirectHandler.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaoh <jaoh@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/03 17:53:37 by jaoh              #+#    #+#             */
/*   Updated: 2026/02/08 14:40:21 by jaoh             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REDIRECTHANDLER_HPP
#define REDIRECTHANDLER_HPP

#include "HttpResponse.hpp"
#include <string>

class RedirectHandler {
public:
    static HttpResponse build(int code, const std::string& target) {
        HttpResponse r;
        r.setStatus(code);                 // 301/302/307/308 등
        r.setHeader("Location", target);
        r.setContentType("text/html");
        r.setBody("<html><body>Redirecting...</body></html>");
        return r;
    }
};

#endif
