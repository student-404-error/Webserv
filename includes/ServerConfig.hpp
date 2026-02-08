/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: princessj <princessj@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/27 17:31:12 by jihyeki2          #+#    #+#             */
/*   Updated: 2026/02/08 07:39:18 by princessj        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include "Token.hpp" // <string>은 Token.hpp에 있음
#include "LocationConfig.hpp"
#include "ConfigException.hpp"
#include <vector>
#include <stdexcept>

class	ServerConfig
{
	public:
		ServerConfig(void);
		~ServerConfig(void);

    		/* getter */
		const std::vector<int>& getListenPorts(void) const;
		const std::vector<LocationConfig>& getLocations(void) const;
		const std::string&				getRoot(void) const;
		const std::string&				getErrorPage(void) const;
		bool							hasMethods(void) const;
		const std::vector<std::string>&	getMethods(void) const;
		/* TODO: 가상호스트용 필드/게터 추가 예정
			- std::vector<std::string> _serverNames;
			- std::string _host; // listen IP 지정 시
		*/
		
		// For testing/manual setup
		void addListenPort(int port);
		void							parseDirective(const std::vector<Token> &tokens, size_t &i);
		void							addLocation(const LocationConfig &location); // location은 server 내부에 종속: ServerConfig가 관리 및 내부에서 통제 가능(캡슐화)
		void							validateServerBlock(void); // server block 전체 보고 판단: 의미적으로 완성되었는가, 기본값을 채워야 하는가

		struct	ListenAddress
		{
			std::string	ip;
			int			port;
		};
	
	private:
		/* semantic handlersfuncs */
		void	handleListen(const std::vector<Token> &tokens, size_t &i);
		void	handleRoot(const std::vector<Token> &tokens, size_t &i);
		void	handleErrorPage(const std::vector<Token> &tokens, size_t &i);
		void	handleMethods(const std::vector<Token>& tokens, size_t& i);

		void	duplicateLocationPathCheck(void) const;
		void	applyDefaultErrorPage(void);
		void	applyDefaultRoot(void);
		void	validateListenDirective(void) const;
		void	checkDuplicateListen(const std::string& ip, int port) const;
	
		/* 대표적인 server level 필드 - 추후 더 추가 예정 (TODO) 회의 해야함) */
		std::vector<ListenAddress>	_listen;
		std::string					_root;
		std::string					_errorPage; // error page path
		std::vector<LocationConfig>	_locations; // location 중첩 구조: location은 항상 server에 속함, server가 vector로 관리

		/* methods */
		std::vector<std::string>	_methods;
		bool						_hasMethods;
};

#endif
