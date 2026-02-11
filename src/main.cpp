#include "parsing.hpp"
#include "HttpApp.hpp"
#include "net/HttpServer.hpp"
#include "middleware/LocationRouter.hpp"
#include "middleware/BodySizeValidationMiddleware.hpp"
#include "middleware/MethodFilterMiddleware.hpp"
#include "middleware/RedirectMiddleware.hpp"
#include "middleware/UploadMiddleware.hpp"
#include "middleware/StaticFileMiddleware.hpp"
#include "middleware/NotFoundMiddleware.hpp"

#include <iostream>
#include <memory>

int main(int argc, char** argv) {
	std::string configPath = "config/test.conf";
	if (argc > 1)
		configPath = argv[1];

	try {
		Config config(configPath);

		for (const auto& serverCfg : config.getServers()) {
			auto cfg = std::make_unique<ServerConfig>(serverCfg);

			// Build the middleware pipeline  (order matters!)
			//
			//  Request
			//    │
			//    ▼
			//  1. LocationRouter   ── picks best location by longest-prefix match
			//    │
			//    ▼
			//  2. BodySizeValidationMiddleware ── rejects requests exceeding client_max_body_size (413)
			//    │
			//    ▼
			//  3. MethodFilterMiddleware     ── rejects disallowed HTTP methods  (405)
			//    │
			//    ▼
			//  4. RedirectMiddleware  ── returns 3xx if location has "return"
			//    │
			//    ▼
			//  5. UploadMiddleware    ── handles POST/DELETE to upload locations
			//    │
			//    ▼
			//  6. CgiHandler       ── executes CGI scripts (.py, .php, …)
			//    │
			//    ▼
			//  7. StaticFileMiddleware ── serves files, index, autoindex
			//    │
			//    ▼
			//  8. NotFoundMiddleware ── send error response
			//
			auto app = std::make_unique<HttpApp>(*cfg);
			app->use(std::make_unique<LocationRouter>(cfg->locations));
			app->use(std::make_unique<BodySizeValidationMiddleware>(*cfg));
			app->use(std::make_unique<MethodFilterMiddleware>());
			app->use(std::make_unique<RedirectMiddleware>());
			app->use(std::make_unique<UploadMiddleware>());
			// app->use(std::make_unique<CgiHandler>());
			app->use(std::make_unique<StaticFileMiddleware>());
			app->use(std::make_unique<NotFoundMiddleware>());

			auto server = std::make_unique<HttpServer>(std::move(cfg));
			server->setApp(std::move(app));

			std::cout << "Starting server '" << serverCfg.server_name
			          << "' on ports:";
			for (const auto& p : serverCfg.listen_ports)
				std::cout << " " << p;
			std::cout << std::endl;

			server->run(); // blocks — for multiple servers you'd use threads
		}
	} catch (const std::exception& e) {
		std::cerr << "Fatal: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
