/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ErrorHandler.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaoh <jaoh@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/04 17:14:21 by jaoh              #+#    #+#             */
/*   Updated: 2026/02/12 01:02:38 by jaoh             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ERRORHANDLER_HPP
#define ERRORHANDLER_HPP

#include "HttpResponse.hpp"
#include <string>
#include <map>

// 에러 응답을 생성하고 커스텀 에러 페이지를 제공하는 핸들러
class ErrorHandler {
public:
    // 기본 에러 응답 생성
    static HttpResponse buildError(int code);
    
    // 커스텀 에러 페이지를 사용한 에러 응답 생성
    static HttpResponse buildError(int code,
                                   const std::map<int, std::string>& errorPages);

private:
    // 기본 에러 HTML 템플릿
    static std::string getDefaultErrorPage(int code, const std::string& message);
    
    // 파일에서 커스텀 에러 페이지 읽기
    static std::string readErrorPage(const std::string& path);
    
    // 상태 코드에 대한 설명 메시지
    static std::string getErrorMessage(int code);
};

#endif
