/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaoh <jaoh@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/20 16:27:19 by jaoh              #+#    #+#             */
/*   Updated: 2026/02/08 14:53:31 by jaoh             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <string>
#include <map>

class HttpResponse {
public:
    HttpResponse();

    void setStatus(int code);
    void setHeader(const std::string& key, const std::string& value);
    void setContentType(const std::string& type);
    void setBody(const std::string& body);

    std::string toString();

private:
    std::string getStatusMessage(int code) const;
    void ensureHeaders();

private:
    int statusCode;
    std::string statusMessage;
    std::map<std::string, std::string> headers;
    std::string body;
};

#endif
